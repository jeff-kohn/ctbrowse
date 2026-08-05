#include "DatasetMgrAsyncImpl.h"
#include "async/senders.h"

#include <asio/awaitable.hpp>
#include <asio/read.hpp>
#include <asio/use_awaitable.hpp>
#include <exec/static_thread_pool.hpp>
#include <fmt/std.h>

namespace ctb
{
   using namespace senders;


   //inline auto injectCancelToken(DatasetMgrAsyncImpl::CancelToken& cancel_token)
   //{
   //   return stdexec::write_env(stdexec::prop(stdexec::get_stop_token, cancel_token));
   //}



   DatasetMgrAsyncImpl& DatasetMgrAsyncImpl::setTableFolder(const std::string& folder)
   {
      m_table_folder = folder;
      return *this;
   }


   const std::string& DatasetMgrAsyncImpl::getTableFolder() const
   {
      return m_table_folder;
   }


   DatasetMgrAsyncImpl& DatasetMgrAsyncImpl::setLabelImageFolder(const std::string& folder)
   {
      m_label_folder = folder;
      return *this;
   }


   const std::string& DatasetMgrAsyncImpl::getLabelImageFolder() const
   {
      return m_label_folder;
   }


   void DatasetMgrAsyncImpl::downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableResultCallback notify_callback)
   {

      auto http_pipeline = [this, table_id, callback = move(notify_callback)](HttpDownloader::HttpResult result) mutable
      {
         auto process = just(move(result))
                      | continues_on(m_cpu_pool.get_scheduler())
                      | then(validateHttpResponse)
                      | then(
                           [table_id](auto response)
                           {
                              return createRawTableFromResponse(response, table_id);
                           })
                      | then(convertTableToUtf8)

                      | continues_on(m_io_pool.get_scheduler())
                      | let_value(
                           [this](RawTableData& table) mutable
                           {
                              std::string table_path = getTablePath(m_table_folder, table.table_id).generic_string();
                              return m_io_pool.sndWriteFile(move(table_path), table.data);   // safe - let_value guarantees lifetime of table
                           })

                      | continues_on(m_cpu_pool.get_scheduler())
                      | then(
                           [table_id, callback]([[maybe_unused]] IoManager::WriteFileResult bytes_written) mutable
                           {
                              SPDLOG_DEBUG("Successfully downloaded table '{}'.", getTableDescription(table_id));
                              callback(TableDownloadInfo{ .table_id = table_id, .file_size = bytes_written.value_or(0) });
                           })

                      | let_error(
                           [this, callback](std::exception_ptr ep) mutable noexcept
                           {
                              // returns a nested error pipeline that ensures callback happens on cpu_pool and doesn't throw
                              return safeErrorCallback(callback, ep);
                           });


         // Launch the processing pipeline asynchronously.
         m_job_scope.spawn(move(process));
      };

      // the whole operation starts here with async HTTP client request, which executes the above lambda as completion callback
      m_http_client.downloadTable(table_id, cred, move(http_pipeline));
   }


   void DatasetMgrAsyncImpl::retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback)
   {
      auto getImageContents = [this](uint64_t wine_id)
      {
         return sndGetLabelImage(wine_id);
      };

      // NOLINTNEXTLINE [cppcoreguidelines-avoid-reference-coroutine-parameters]	because it's safe with let_value()
      auto saveIfNeeded = [this](ImageFileContents& image_contents) mutable -> exec::task<ImageFileContents>
      {
         // if url is present, we downloaded the file so go ahead and save it (overwrite would be unlikely but OK)
         if (image_contents.url.has_value())
         {
            auto bytes_written = co_await m_io_pool.sndWriteFile(image_contents.file_path.generic_string(), image_contents.contents);

            if (!bytes_written) throw Error(bytes_written.error());
            SPDLOG_DEBUG("CtDatasetMgr::retrieveLabelImageAsync - saved {} bytes to '{}'", bytes_written.value(), image_contents.file_path);
         }
         // forward the data to the next sender
         co_return move(image_contents);
      };

      auto errorCallback = [this, result_callback](std::exception_ptr& ep) mutable noexcept
      {
         return safeErrorCallback(move(result_callback), move(ep));
      };

      auto pipeline =
         just(wine_id)
         | continues_on(m_io_pool.get_scheduler())
         | let_value(getImageContents)
         | let_value(saveIfNeeded)
         | continues_on(m_cpu_pool.get_scheduler())
         | then(result_callback)
         | stopped_as_error(std::make_exception_ptr(Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled }))
         | let_error(errorCallback);

      m_job_scope.spawn(move(pipeline));
   }


   void DatasetMgrAsyncImpl::checkBrowserLoginAsync(CredentialWrapper cred, LoginResultCallback result_callback)
   {
      auto doLogin = [this](CredentialWrapper& cred)
      {
         return m_browser.sndAttemptLogin(move(cred));
      };
      auto errorCallback = [this, result_callback](std::exception_ptr& ep) noexcept
      {
         return safeErrorCallback(move(result_callback), move(ep));
      };

      auto pipeline =
         just(move(cred))
         | continues_on(m_io_pool.get_scheduler())
         | let_value(doLogin)
         | continues_on(m_cpu_pool.get_scheduler())
         | then(result_callback)   // NOLINT [clang-analyzer-cplusplus.NewDeleteLeaks]
         | stopped_as_error(std::make_exception_ptr(Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled }))
         | let_error(errorCallback);

      m_job_scope.spawn(move(pipeline));
   }


   void DatasetMgrAsyncImpl::startBrowser(std::string browser_path, std::string data_dir, int32_t port)
   {
      m_browser.start(browser_path, data_dir, port);
   }


   bool DatasetMgrAsyncImpl::requestShutdown()
   {
      if (shutdownRequested()) return true;

      m_job_scope.request_stop();
      m_browser.stop();
      return true;
   }


   bool DatasetMgrAsyncImpl::shutdownRequested() const
   {
      // there should be a const-friendly stop_requested() on the scope object, then we wouldn't need the cast...
      return const_cast<exec::async_scope&>(m_job_scope).get_stop_source().stop_requested();
   }


   DatasetMgrAsyncImpl::~DatasetMgrAsyncImpl() noexcept
   {
      try
      {
         requestShutdown();
         m_browser.stop();
      }
      catch(...){} // NOLINT
   }


   bool DatasetMgrAsyncImpl::waitForShutdown()
   {
      if (!shutdownRequested()) return false;

      stdexec::sync_wait(m_job_scope.on_empty());
      return true;
   }


   void DatasetMgrAsyncImpl::throwIfShuttingDown() noexcept(false)
   {
      if (shutdownRequested()) throw Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled };
   }


   /// @brief Retrieve the requested label image
   ///
   /// The file will be read from disk if it exists on cache, otherwise it will be
   /// downloaded from CellarTracker.com
   /// @param wine_id -  the id of the wine to retrieve an image for
   /// @return - struct containing the image file's contents as well as some metadata
   /// @throw - ctb::Error if file couldn't be read from disk or downloaded
   exec::task<ImageFileContents> DatasetMgrAsyncImpl::sndGetLabelImage(uint64_t wine_id) noexcept(false)
   {
      throwIfShuttingDown();

      ImageFileContents retval{ .wine_id = wine_id };

      // try to read the file locally first.
      auto local_path         = buildLabelPath(m_label_folder, wine_id);
      auto local_cache_result = co_await m_io_pool.sndReadFile(local_path);
      if (local_cache_result)
      {
         retval.file_path = move(local_path);
         retval.contents  = move(*local_cache_result);

         co_return retval;
      }

      SPDLOG_DEBUG("CtDatasetMgr::AsyncImpl::sndGetLabelImage couldn't read local file '{}', will attempt to download label.", local_path);
      HttpFileContents file_contents = co_await m_browser.sndDownloadLabel(wine_id);
      retval.url                     = file_contents.original_url;
      retval.file_path               = local_path;

      // stdexec::on is a roundtrip scheduler, this will get executed on cpu_pool and then we'll continue on original scheduler.
      retval.contents = co_await stdexec::on(m_cpu_pool.get_scheduler(), just(move(file_contents)) | then(decodeResourceContents));

      co_return retval;
   }

}   // namespace ctb


