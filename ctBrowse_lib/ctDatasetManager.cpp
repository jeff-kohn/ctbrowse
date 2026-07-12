#include "ctb/model/CtDatasetManager.h"

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

namespace ctb::app
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


   // private impl details for CtDatasetManager, to prevent public header dependencies.
   struct CtDatasetManager::AsyncImpl
   {
      static constexpr uint32_t NUM_CPU_THREADS = 2;
      static constexpr uint32_t NUM_IO_THREADS  = 1;

      // ordering is important here not only for initialization but also teardown
      web::IoManager            io_pool{};
      exec::static_thread_pool  cpu_pool{ NUM_CPU_THREADS };
      HttpDownloader            http_client{ io_pool.get_executor() };
      web::CallarTrackerBrowser browser{ io_pool.get_context() };
   };


   CtDatasetManager::CtDatasetManager()
   {}


   CtDatasetManager::~CtDatasetManager() noexcept
   {
      m_impl->cpu_pool.request_stop();
   }


   CtDatasetManager::CtDatasetManager(const fs::path& table_folder) noexcept(false)
   {
      setTableFolder(table_folder);
   }


   auto CtDatasetManager::loadDataset(TableId table_id) -> DatasetPtr
   {
      switch (table_id)
      {
         case TableId::List        : return getDatasetOrThrow<WineListTable>(m_table_folder, table_id);
         case TableId::Pending     : return getDatasetOrThrow<PendingWineTable>(m_table_folder, table_id);
         case TableId::Consumed    : return getDatasetOrThrow<ConsumedWineTable>(m_table_folder, table_id);
         case TableId::Availability: return getDatasetOrThrow<ReadyToDrinkTable>(m_table_folder, table_id);
         case TableId::Purchase    : return getDatasetOrThrow<PurchasedWineTable>(m_table_folder, table_id);
         case TableId::Tag         : return getDatasetOrThrow<TaggedWinesTable>(m_table_folder, table_id);
         case TableId::Inventory   : return getDatasetOrThrow<BottleInventoryTable>(m_table_folder, table_id);
         case TableId::PrivateNotes: return getDatasetOrThrow<PrivateNotesTable>(m_table_folder, table_id);
         case TableId::Notes       : return getDatasetOrThrow<TastingNotesTable>(m_table_folder, table_id);
         default                   : throw Error{ "Table not found." };
      };
   }


   /// @brief Load a dataset and apply options
   auto CtDatasetManager::loadDataset(const CtDatasetOptions& options) -> DatasetPtr
   {
      // load dataset and then apply options.
      auto dataset = loadDataset(options.table_id);
      options.applyToDataset(dataset);
      return dataset;
   }


   auto CtDatasetManager::getProReviewsCache() -> ProReviewsCache&
   {
      if (!m_pro_cache)
      {
         m_pro_cache = loadTableData<ProReviewsCacheTable>(m_table_folder, TableId::Availability).value_or({});
      }
      return m_pro_cache.value();
   }


   void CtDatasetManager::downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableResultCallback result_callback)
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
                           [table_id, callback]([[maybe_unused]] web::IoManager::WriteFileResult bytes_written) mutable
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


   void CtDatasetManager::retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback)
   {
      auto* io_ptr = &(m_impl->io_pool);

      auto attemptLocalRead = std::bind_front(&web::IoManager::sndReadFile, io_ptr);

      auto downloadIfMissing = [this, wine_id](web::IoManager::ReadFileResult& cache_file) -> exec::task<ImageResult>
      {
         Buffer image_bytes{};
         if (cache_file)
         {
            image_bytes.swap(*cache_file);
         }
         else
         {
            SPDLOG_DEBUG("Label image for wine id '{}' not found in local cache, attempting download from CT.com", wine_id);
            //image_bytes        = co_await m_impl->browser.sndDownloadLabel(wine_id);
            auto bytes_written = co_await m_impl->io_pool.sndWriteFile(buildLabelPath(getLabelImageFolder(), wine_id), image_bytes);
         }
         co_return image_bytes;
      };
      //| stopped_as_error(std::make_exception_ptr(Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled }))

      auto pipeline = just(buildLabelPath(getLabelImageFolder(), wine_id))
                    | continues_on(m_impl->io_pool.get_scheduler())
                    | let_value(attemptLocalRead)
                    | let_value(downloadIfMissing)
                    | continues_on(m_impl->cpu_pool.get_scheduler())
                    | then(
                         [result_callback](ImageResult result)
                         {
                            result_callback(move(result));
                         })
                    | let_stopped(
                         [this, result_callback]() mutable noexcept
                         {
                            return safeErrorCallback(
                               m_impl->cpu_pool.get_scheduler(),
                               result_callback,
                               std::make_exception_ptr(Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled }));
                         })
                    | let_error(
                         [this, result_callback](std::exception_ptr ep) mutable noexcept
                         {
                            return safeErrorCallback(m_impl->cpu_pool.get_scheduler(), result_callback, ep);
                         });

      start_detached(move(pipeline));
   }


   auto CtDatasetManager::setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetManager&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
      }
      m_table_folder = folder;
      return *this;
   }


   /// @brief returns the location used for loading data files from disk
   auto CtDatasetManager::getTableFolder() const -> const fs::path&
   {
      return m_table_folder;
   }


   auto CtDatasetManager::setLabelImageFolder(const fs::path& folder) noexcept(false) -> CtDatasetManager&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
      }
      m_label_folder = folder;
      return *this;
   }


   auto CtDatasetManager::getLabelImageFolder() const -> const fs::path&
   {
      return m_label_folder;
   }


}   // namespace ctb::app
