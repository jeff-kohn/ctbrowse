#include "ctb/model/CtDatasetManager.h"

#include "ctb/HttpDownloader.h"
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
#include "ctb/tasks/async_tasks.h"

#include <exec/static_thread_pool.hpp>
#include <exception>


namespace ctb::app
{
   namespace
   {
      /// @brief Attempts to load a dataset from file path, throws on error.
      template<typename TableT>
      auto getDatasetOrThrow(const fs::path& folder, TableId tbl_id) -> DatasetPtr
      {
         auto result = loadTableData<TableT>(folder, tbl_id);
         if (!result) throw Error{ result.error() };

         return CtDataset<TableT>::create(std::move(result.value()));
      }

   }   // namespace


   // private impl details for CtDatasetManager, to prevent public header dependencies.
   struct CtDatasetManager::AsyncImpl
   {
      static constexpr uint32_t NUM_CPU_THREADS = 2;
      static constexpr uint32_t NUM_IO_THREADS  = 1;

      exec::static_thread_pool     cpu_pool{ NUM_CPU_THREADS };
      exec::asio::asio_thread_pool io_pool{ NUM_IO_THREADS };
      HttpDownloader               http_client{ io_pool.get_executor() };
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
         case TableId::List        : return getDatasetOrThrow<WineListTable>(m_data_folder, table_id);
         case TableId::Pending     : return getDatasetOrThrow<PendingWineTable>(m_data_folder, table_id);
         case TableId::Consumed    : return getDatasetOrThrow<ConsumedWineTable>(m_data_folder, table_id);
         case TableId::Availability: return getDatasetOrThrow<ReadyToDrinkTable>(m_data_folder, table_id);
         case TableId::Purchase    : return getDatasetOrThrow<PurchasedWineTable>(m_data_folder, table_id);
         case TableId::Tag         : return getDatasetOrThrow<TaggedWinesTable>(m_data_folder, table_id);
         case TableId::Inventory   : return getDatasetOrThrow<BottleInventoryTable>(m_data_folder, table_id);
         case TableId::PrivateNotes: return getDatasetOrThrow<PrivateNotesTable>(m_data_folder, table_id);
         case TableId::Notes       : return getDatasetOrThrow<TastingNotesTable>(m_data_folder, table_id);
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


   auto CtDatasetManager::getProReviewsCache() -> std::optional<ProReviewsCache>
   {
      return std::optional<ProReviewsCache>();
   }


   void CtDatasetManager::downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableDownloadResultCallback notify_callback)
   {
      auto http_pipeline = [this, table_id, callback = std::move(notify_callback)](HttpDownloader::HttpResult result) mutable
      {
         // keep namespace pollution out of the expression, especially since stdexec will probably become std::exec
         using namespace ctb::tasks;

         // these will go out of scope when the task is started asynchronously, need to movee/copied into lambdas, never capture by reference!
         auto              target_path = getTablePath(getTableFolder(), table_id).generic_string();
         asio::stream_file file{ m_impl->io_pool.get_executor(), target_path,
                                 asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate };

         auto process =
            just(std::move(result))

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
               [file = std::move(file)](RawTableData& table) mutable
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
                  callback(std::move(msg));
               })

            // this could be on either scheduler depending on which step threw an exception, so we use a nested error pipeline
            // to ensure callback safely happens on cpu_pool
            | let_error(
               [this, callback](std::exception_ptr ep) mutable noexcept
               {
                  return safeErrorCallback(m_impl->cpu_pool.get_scheduler(), callback, ep);
               });

         // Launch the processing pipeline asynchronously.
         start_detached(std::move(process));
      };

      // the whole operation starts here with async HTTP client request, which executes the above labmda as completion callback
      m_impl->http_client.downloadTable(table_id, cred, std::move(http_pipeline));
   }


   auto CtDatasetManager::setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetManager&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DataError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
      }
      m_data_folder = folder;
      return *this;
   }


   /// @brief returns the location used for loading data files from disk
   auto CtDatasetManager::getTableFolder() const -> const fs::path&
   {
      return m_data_folder;
   }


}   // namespace ctb::app
