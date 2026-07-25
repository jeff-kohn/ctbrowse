#include "ctb/model/CtDatasetMgr.h"

#include "CellarTrackerBrowser.h"
#include "HttpDownloader.h"
#include "IoManager.h"
#include "senders.h"

#include "ctb/model/CtDataset.h"
#include "ctb/model/ProReviewsCache.h"
#include "ctb/tables/BottleInventoryTraits.h"
#include "ctb/tables/ConsumedWineTraits.h"
#include "ctb/tables/PendingWineTraits.h"
#include "ctb/tables/PrivateNotesTraits.h"
#include "ctb/tables/PurchasedWineTraits.h"
#include "ctb/tables/ReadyToDrinkTraits.h"
#include "ctb/tables/TaggedWinesTraits.h"
#include "ctb/tables/TastingNotesTraits.h"
#include "ctb/tables/WineListTraits.h"
#include "ctb/tables/table_data.h"

#include <asio/awaitable.hpp>
#include <asio/read.hpp>
#include <asio/use_awaitable.hpp>
#include <exec/static_thread_pool.hpp>
#include <fmt/std.h>

#include <exception>

namespace ctb
{
   using namespace ctb::tasks;

   namespace
   {

#ifndef ERROR_FILE_NOT_FOUND
      constexpr long ERROR_FILE_NOT_FOUND = 2L;
#endif   // !ERROR_FILE_NOT_FOUND


      /// @brief Attempts to load a dataset from file path, throws on error.
      template<typename TableT>
      auto getDatasetOrThrow(const fs::path& folder, TableId tbl_id) -> DatasetPtr
      {
         auto result = loadTableData<TableT>(folder, tbl_id);
         if (!result) throw Error{ result.error() };

         return CtDataset<TableT>::create(move(result.value()));
      }

   }   // namespace


   // private impl details for CtDatasetMgr, to prevent public header dependencies.
   struct CtDatasetMgr::AsyncImpl
   {
      static constexpr uint32_t NUM_CPU_THREADS = 2;
      static constexpr uint32_t NUM_IO_THREADS  = 1;

      AsyncImpl() = default;   // todo need to start browser, which means we need browser data dir.

      // ordering is important here not only for initialization but also teardown
      fs::path                 label_folder{ ctb::format("{}/Labels", constants::CURRENT_DIRECTORY) };
      fs::path                 table_folder{ constants::CURRENT_DIRECTORY };
      IoManager                io_pool{};
      exec::static_thread_pool cpu_pool{ NUM_CPU_THREADS };
      HttpDownloader           http_client{ io_pool.get_executor() };
      CellarTrackerBrowser     browser{ io_pool.get_context() };

      /// @brief Get the file contents for the specified wine label.
      exec::task<ImageFileContents> sndGetLabelImage(uint64_t wine_id) noexcept(false);
   };


   /// @brief Retrieve the requested label image
   ///
   /// The file will be read from disk if it exists on cache, otherwise it will be
   /// downloaded from CellarTracker.com
   /// @param wine_id -  the id of the wine to retrieve an image for
   /// @return - struct containing the image file's contents as well as some metadata
   /// @throw - ctb::Error if file couldn't be read from disk or downloaded
   exec::task<ImageFileContents> CtDatasetMgr::AsyncImpl::sndGetLabelImage(uint64_t wine_id) noexcept(false)
   {
      ImageFileContents retval{ .wine_id = wine_id };

      // try to read the file locally first.
      auto local_path         = buildLabelPath(label_folder, wine_id);
      auto local_cache_result = co_await io_pool.sndReadFile(local_path);
      if (local_cache_result)
      {
         retval.file_path = std::move(local_path);
         retval.data      = std::move(*local_cache_result);

         co_return retval;
      }

      SPDLOG_DEBUG("CtDatasetMgr::AsyncImpl::sndGetLabelImage couldn't read local file '{}', will attempt to download label.", local_path);
      HttpFileContents file_contents = co_await browser.downloadLabel(wine_id);

      // stdexec::on is a roundtrip scheduler, this will get executed on cpu_pool and then continue on our original scheduler.
      retval.data = co_await stdexec::on(cpu_pool.get_scheduler(), just(move(file_contents)) | then(decodeResourceContents));

      co_return retval;
   }


   CtDatasetMgr::CtDatasetMgr()
   {}


   CtDatasetMgr::~CtDatasetMgr() noexcept
   {
      m_impl->cpu_pool.request_stop();
   }


   ctb::CtDatasetMgr::CtDatasetMgr(const DatasetMgrOptions opts) noexcept(false)
   {
      if (!opts.browser_path.empty())
      {
         m_impl->browser.start(opts.browser_path, opts.browser_data_dir, opts.browser_ws_port);
      }
      setTableFolder(opts.table_folder);
      setLabelImageFolder(opts.label_folder);
   }


   auto CtDatasetMgr::loadDataset(TableId table_id) -> DatasetPtr
   {
      switch (table_id)
      {
         case TableId::List        : return getDatasetOrThrow<WineListTable>(m_impl->table_folder, table_id);
         case TableId::Pending     : return getDatasetOrThrow<PendingWineTable>(m_impl->table_folder, table_id);
         case TableId::Consumed    : return getDatasetOrThrow<ConsumedWineTable>(m_impl->table_folder, table_id);
         case TableId::Availability: return getDatasetOrThrow<ReadyToDrinkTable>(m_impl->table_folder, table_id);
         case TableId::Purchase    : return getDatasetOrThrow<PurchasedWineTable>(m_impl->table_folder, table_id);
         case TableId::Tag         : return getDatasetOrThrow<TaggedWinesTable>(m_impl->table_folder, table_id);
         case TableId::Inventory   : return getDatasetOrThrow<BottleInventoryTable>(m_impl->table_folder, table_id);
         case TableId::PrivateNotes: return getDatasetOrThrow<PrivateNotesTable>(m_impl->table_folder, table_id);
         case TableId::Notes       : return getDatasetOrThrow<TastingNotesTable>(m_impl->table_folder, table_id);
         default                   : throw Error{ "Table not found." };
      };
   }


   /// @brief Load a dataset and apply options
   auto CtDatasetMgr::loadDataset(const CtDatasetOptions& options) -> DatasetPtr
   {
      // load dataset and then apply options.
      auto dataset = loadDataset(options.table_id);
      options.applyToDataset(dataset);
      return dataset;
   }


   auto CtDatasetMgr::getProReviewsCache() -> ProReviewsCache&
   {
      if (!m_pro_cache)
      {
         m_pro_cache = loadTableData<ProReviewsCacheTable>(m_impl->table_folder, TableId::Availability).value_or({});
      }
      return m_pro_cache.value();
   }


   void CtDatasetMgr::downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableResultCallback result_callback)
   {

      auto http_pipeline = [this, table_id, callback = move(result_callback)](HttpDownloader::HttpResult result) mutable
      {
         auto process = just(move(result))
                      | continues_on(m_impl->cpu_pool.get_scheduler())
                      | then(validateHttpResponse)
                      | then(
                           [table_id](auto response)
                           {
                              return createRawTableFromResponse(response, table_id);
                           })
                      | then(convertTableToUtf8)

                      | continues_on(m_impl->io_pool.get_scheduler())
                      | let_value(
                           [this](RawTableData& table) mutable
                           {
                              return m_impl->io_pool.sndWriteFile(getTablePath(getTableFolder(), table.table_id), table.data);
                           })

                      | continues_on(m_impl->cpu_pool.get_scheduler())
                      | then(
                           [table_id, callback]([[maybe_unused]] IoManager::WriteFileResult bytes_written) mutable
                           {
                              auto msg = format("Successfully downloaded table '{}'.", getTableDescription(table_id));
                              SPDLOG_DEBUG(msg);
                              callback(move(msg));
                           })

                      // this could be on either scheduler depending on which step threw an exception, so we use a nested error pipeline
                      // to ensure callback safely happens on cpu_pool
                      | let_error(
                           [this, callback](std::exception_ptr ep) mutable noexcept
                           {
                              return safeErrorCallback(m_impl->cpu_pool.get_scheduler(), callback, ep);
                           });

         // Launch the processing pipeline asynchronously.
         start_detached(move(process));
      };

      // the whole operation starts here with async HTTP client request, which executes the above labmda as completion callback
      m_impl->http_client.downloadTable(table_id, cred, move(http_pipeline));
   }


   void CtDatasetMgr::retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback)
   {
      auto getImageContents = std::bind_front(&AsyncImpl::sndGetLabelImage, &(*m_impl));

      auto saveIfNeeded = [this](ImageFileContents image_contents) -> exec::task<ImageFileContents>
      {
         // if url is present, we downloaded the file so go ahead and save it (overwrite would be unlikely but OK)
         if (image_contents.url.has_value())
         {
            auto [[maybe_unused]] bytes_written = co_await m_impl->io_pool.sndWriteFile(image_contents.file_path, image_contents.data);
            SPDLOG_DEBUG("CtDatasetMgr::retrieveLabelImageAsync - saved {} bytes to '{}'", bytes_written, image_contents.file_path);
         }
         // just forward the data to the next sender
         co_return image_contents;
      };

      auto errorCallback = [this, result_callback](std::exception_ptr ep) noexcept
      {
         return safeErrorCallback(m_impl->cpu_pool.get_scheduler(), result_callback, ep);
      };

      auto pipeline =
         just(wine_id)
         | continues_on(m_impl->io_pool.get_scheduler())
         | let_value(getImageContents)
         | let_value(saveIfNeeded)
         | continues_on(m_impl->cpu_pool.get_scheduler())
         | then(result_callback)
         | stopped_as_error(std::make_exception_ptr(Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled }))
         | let_error(errorCallback);

      start_detached(move(pipeline));
   }


   auto CtDatasetMgr::setTableFolder(const std::string& folder) noexcept(false) -> CtDatasetMgr&
   {
      fs::path folder_path{ expandEnvironmentVars(folder) };
      if (!fs::exists(folder_path) and !createFolderPath(folder_path))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_PATH_NOT_FOUND,
                      folder_path.generic_string() };
      }
      m_impl->label_folder = folder_path;
      return *this;
   }


   auto CtDatasetMgr::setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetMgr&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
      }
      m_impl->label_folder = folder;
      return *this;
   }


   auto CtDatasetMgr::getTableFolder() const -> const fs::path&
   {
      return m_impl->table_folder;
   }


   auto CtDatasetMgr::setLabelImageFolder(const std::string& folder) noexcept(false) -> CtDatasetMgr&
   {
      fs::path folder_path{ expandEnvironmentVars(folder) };
      if (!fs::exists(folder_path) and !createFolderPath(folder_path))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER,
                      folder_path.generic_string() };
      }
      m_impl->label_folder = folder_path;
      return *this;
   }


   auto CtDatasetMgr::setLabelImageFolder(const fs::path& folder) noexcept(false) -> CtDatasetMgr&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER,
                      folder.generic_string() };
      }
      m_impl->label_folder = folder;
      return *this;
   }


   auto CtDatasetMgr::getLabelImageFolder() const -> const fs::path&
   {
      return m_impl->label_folder;
   }


}   // namespace ctb
