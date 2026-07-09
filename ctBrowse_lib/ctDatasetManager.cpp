#include "ctb/model/CtDatasetManager.h"

#include "AsioThreadScheduler.h"
#include "HeadlessBrowser.h"
#include "HttpDownloader.h"
#include "async_tasks.h"

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
      constexpr long FILE_NOT_FOUND = 2L;
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
      AsioThreadScheduler      io_pool{};
      exec::static_thread_pool cpu_pool{ NUM_CPU_THREADS };
      HttpDownloader           http_client{ io_pool.get_executor() };
      web::HeadlessBrowser     m_web_client{ io_pool.get_context() };
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
         // these will go out of scope when the task is started asynchronously, need to move/copy into lambdas, never capture by reference!
         auto              target_path = getTablePath(getTableFolder(), table_id).generic_string();
         asio::stream_file file{ m_impl->io_pool.get_executor(), target_path,
                                 asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate };

         auto process = just(move(result))

                      // validation/conversion happens on cpu scheduler to keep I/O thread free
                      | continues_on(m_impl->cpu_pool.get_scheduler())
                      | then(validateHttpResponse)
                      | then(
                           [table_id](auto response)
                           {
                              return createRawTableFromResponse(response, table_id);
                           })
                      | then(convertTableToUtf8)

                      // back to I/O scheduler to save the data to disk file.
                      | continues_on(m_impl->io_pool.get_scheduler())
                      | let_value(
                           [file = move(file)](RawTableData& table) mutable
                           {
                              return asio::async_write(file, asio::buffer(table.data), use_sender);
                           })

                      // then back again to cpu scheduler for callback notification
                      | continues_on(m_impl->cpu_pool.get_scheduler())
                      | then(
                           [table_id, callback]([[maybe_unused]] std::size_t bytes_written) mutable
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
      //      auto attemptLocalRead = std::bind_front(&AsyncImpl::coroReadFile, &(*m_impl));

      auto downloadIfMissing = [this, wine_id](ImageResult& image_result)
      {
         //if (image_result)
         //{
         //   co_return move(image_result);
         //}
         //auto session = co_await m_impl->m_web_client.coroDownloadImage();
      };

      //auto pipeline = just(buildLabelPath(getLabelImageFolder(), wine_id)) | let_value(attemptLocalRead) | let_value(downloadIfMissing);
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
