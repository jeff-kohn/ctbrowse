#include "ctb/model/ctDatasetManager.h"

#include "BackgroundThreadContext.h"

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
#include "ctb/utility_http.h"

#include <asio/buffer.hpp>
#include <asio/write.hpp>
#include <exec/asio/asio_thread_pool.hpp>
#include <exec/start_detached.hpp>
#include <exec/static_thread_pool.hpp>
#include <stdexec/execution.hpp>

#include <cassert>

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


      /// @brief checks an CellarTracker HTTP response for errors and throws them as Error exceptions
      /// @return the response object from the HttpResult if validation passed
      /// @throw ctb::Error if validation fails
      auto validateResponse(const HttpDownloader::HttpResult response) noexcept(false) -> HttpDownloader::HttpResult::value_type
      {
         if (!response)
         {
            auto&& error = response.error();
            throw ctb::Error{ error.value(), error.message(), Error::Category::HttpStatus };
         }

         if (HttpStatus::isSuccessful(response->status_code))
         {
            // response indicates success, but we need to check if the response contains
            // an error message since CT returns an HTML <body> for auth errors instead of HTTP status code.
            if (response->response_body.starts_with(constants::ERR_STR_INVALID_CELLARTRACKER_LOGON))
            {
               throw ctb::Error{
                  HttpStatus::toInt(HttpStatus::Code::Unauthorized),
                  constants::ERROR_STR_AUTHENTICATION_FAILED,
                  Error::Category::HttpStatus,
               };
            }
         }
         else
         {
            // we received a response, but result code didn't indicate success.
            auto status_msg = HttpStatus::reasonPhrase(response->status_code);

            SPDLOG_DEBUG("HTTP Request received unexpected response: {} - {}. Body starts with {}...",
                         response->status_code,
                         status_msg,
                         response->response_body.substr(0, 128));   // NOLINT

            throw ctb::Error{ response->status_code, status_msg, Error::Category::HttpStatus };
         }
         return *response;
      }


      /// @brief  Converts a glz::response into a RawTableData object
      auto createRawTableFromResponse(HttpDownloader::HttpResult::value_type response, TableId table_id) -> RawTableData
      {
         std::string content_type_header{};
         if (auto it = response.response_headers.find(ctb::headers::CONTENT_TYPE_KEY); it != response.response_headers.end())
         {
            content_type_header = it->second;
         }
         // we got a table (or some sort of body), package the result.
         return RawTableData{ .data        = std::move(response.response_body),
                              .table_id    = table_id,
                              .data_format = DataFormatId::csv,
                              .encoding    = getTextEncodingFromHeader(content_type_header).value_or(TextEncoding::ANSI) };
      }


      /// @brief convert the RawTableData to UTF8 text (if it isn't already)
      auto convertTableToUtf8(RawTableData table_data) -> RawTableData
      {
         // short-circuit check
         if (table_data.encoding == TextEncoding::UTF8) return table_data;

         auto maybe_utf8_text = toUTF8(table_data.data, table_data.encoding);
         if (maybe_utf8_text)
         {
            table_data.data.swap(*maybe_utf8_text);
         }
         return table_data;
      }

   }   // namespace


   // private impl details for ctDatasetManager, to prevent public header dependencies.
   struct ctDatasetManager::AsyncImpl
   {
      static constexpr uint32_t NUM_CPU_THREADS = 2;
      static constexpr uint32_t NUM_IO_THREADS  = 1;

      exec::static_thread_pool     cpu_pool{ NUM_CPU_THREADS };
      exec::asio::asio_thread_pool io_pool{ NUM_IO_THREADS };
      HttpDownloader               http_client{ io_pool.get_executor() };
   };


   ctDatasetManager::ctDatasetManager(const fs::path& folder) noexcept(false)
   {
      setDataFolder(folder);
   }


   //ctDatasetManager::~ctDatasetManager() noexcept // NOLINT needs to be here for AsyncImpl definition.
   //{}


   auto ctDatasetManager::loadDataset(TableId table_id) -> DatasetPtr
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
   auto ctDatasetManager::loadDataset(const CtDatasetOptions& options) -> DatasetPtr
   {
      // load dataset and then apply options.
      auto dataset = loadDataset(options.table_id);
      options.applyToDataset(dataset);
      return dataset;
   }


   auto ctDatasetManager::getProReviewsCache() -> std::optional<ProReviewsCache>
   {
      return std::optional<ProReviewsCache>();
   }


   void ctDatasetManager::downloadTableAsync(TableId table_id, TableDownloadResultCallback notify_callback)
   {
      auto http_pipeline = [this, table_id, callback = std::move(notify_callback)](HttpDownloader::HttpResult result) mutable 
      {
         using stdexec::continues_on;
         using stdexec::just;
         using stdexec::then;
         using stdexec::upon_error;
         using exec::start_detached;

         // note that we're using a single thread for net/disk I/O. If that were to change, we wouldn't be able to declare this 
         // here and just capture by reference below.
         assert(m_impl->NUM_IO_THREADS == 1);
         asio::stream_file file{ m_impl->io_pool.get_executor(), getTablePath(getDataFolder(), table_id).generic_string(),
                                 asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate };

         auto pipeline = just(std::move(result))

                       | continues_on(m_impl->cpu_pool.get_scheduler())
                       | then(validateResponse)
                       | then(
                            [table_id](auto response)
                            {
                               return createRawTableFromResponse(response, table_id);
                            })
                       | then(convertTableToUtf8)

                       | continues_on(m_impl->io_pool.get_scheduler())
                       | then(
                            [&file](RawTableData table)
                            {
                               return asio::async_write(file, asio::buffer(table.data), asio::deferred);
                            })

                       | upon_error(
                            []([[maybe_unused]] std::exception_ptr ep)
                            {
                              SPDLOG_DEBUG(packageError(ep).formattedMessage());
                            });

         // 5. Hop back to IO context for Disk Write
         // ... (your asio::async_write logic here) ...

         // 6. Final UI Notification
         //| stdexec::upon_error([...]) | stdexec::then([...]);

         // Launch the pipeline asynchronously.
         start_detached(std::move(pipeline));
      };

      m_impl->http_client.downloadTable(table_id, m_cred, std::move(http_pipeline));
   }


   auto ctDatasetManager::setDataFolder(const fs::path& folder) noexcept(false) -> ctDatasetManager&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DataError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
      }
      m_data_folder = folder;
      return *this;
   }


   /// @brief returns the location used for loading data files from disk
   auto ctDatasetManager::getDataFolder() const -> const fs::path&
   {
      return m_data_folder;
   }


}   // namespace ctb::app
