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

   /// @brief Retrieve the requested label image
   ///
   /// The file will be read from disk if it exists on cache, otherwise it will be
   /// downloaded from CellarTracker.com
   /// @param wine_id -  the id of the wine to retrieve an image for
   /// @return - struct containing the image file's contents as well as some metadata
   /// @throw - ctb::Error if file couldn't be read from disk or downloaded
   exec::task<ImageFileContents> DatasetMgrAsyncImpl::sndGetLabelImage(uint64_t                    wine_id,
                                                                       stdexec::inplace_stop_token cancel_token) noexcept(false)
   {
      if (cancel_token.stop_requested()) throw Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled };

      ImageFileContents retval{ .wine_id = wine_id };

      // try to read the file locally first.
      auto local_path         = buildLabelPath(label_folder, wine_id);
      auto local_cache_result = co_await io_pool.sndReadFile(local_path);
      if (local_cache_result)
      {
         retval.file_path = std::move(local_path);
         retval.contents  = std::move(*local_cache_result);

         co_return retval;
      }

      SPDLOG_DEBUG("CtDatasetMgr::AsyncImpl::sndGetLabelImage couldn't read local file '{}', will attempt to download label.", local_path);
      HttpFileContents file_contents = co_await browser.downloadLabel(wine_id);

      // stdexec::on is a roundtrip scheduler, this will get executed on cpu_pool and then continue on our original scheduler.
      retval.contents = co_await stdexec::on(cpu_pool.get_scheduler(), just(move(file_contents)) | then(decodeResourceContents));

      co_return retval;
   }


   void DatasetMgrAsyncImpl::downloadTableAsync(TableId                     table_id,
                                                const CredentialWrapper&    cred,
                                                TableResultCallback         result_callback,
                                                stdexec::inplace_stop_token cancel_token)
   {

      auto http_pipeline = [this, table_id, callback = move(result_callback), cancel_token = std::move(cancel_token)](
                              HttpDownloader::HttpResult result) mutable
      {
         auto process = just(move(result))
                      | continues_on(cpu_pool.get_scheduler())
                      | then(validateHttpResponse)
                      | then(
                           [table_id](auto response)
                           {
                              return createRawTableFromResponse(response, table_id);
                           })
                      | then(convertTableToUtf8)

                      | continues_on(io_pool.get_scheduler())
                      | let_value(
                           [this](RawTableData& table) mutable
                           {
                              return io_pool.sndWriteFile(getTablePath(table_folder, table.table_id), table.data);
                           })

                      | continues_on(cpu_pool.get_scheduler())
                      | then(
                           [table_id, callback]([[maybe_unused]] IoManager::WriteFileResult bytes_written) mutable
                           {
                              SPDLOG_DEBUG("Successfully downloaded table '{}'.", getTableDescription(table_id));
                              callback(TableDownloadInfo{ .table_id = table_id, .file_size = bytes_written.value_or(0) });
                           })

                      // this could be on either scheduler depending on which step threw an exception, so we use a nested error pipeline
                      // to ensure callback safely happens on cpu_pool
                      | let_error(
                           [this, callback](std::exception_ptr ep) mutable noexcept
                           {
                              return safeErrorCallback(cpu_pool.get_scheduler(), callback, ep);
                           })

                      | stdexec::write_env(stdexec::prop(stdexec::get_stop_token, move(cancel_token)));

         // Launch the processing pipeline asynchronously.
         start_detached(move(process));
      };

      // the whole operation starts here with async HTTP client request, which executes the above lambda as completion callback
      http_client.downloadTable(table_id, cred, move(http_pipeline));
   }


   void DatasetMgrAsyncImpl::retrieveLabelImageAsync(uint64_t                    wine_id,
                                                     ImageResultCallback         result_callback,
                                                     stdexec::inplace_stop_token cancel_token)
   {
      auto getImageContents = [this, cancel_token](uint64_t wine_id)
      {
         return sndGetLabelImage(wine_id, cancel_token);
      };

      auto saveIfNeeded = [this](ImageFileContents image_contents) -> exec::task<ImageFileContents>
      {
         // if url is present, we downloaded the file so go ahead and save it (overwrite would be unlikely but OK)
         if (image_contents.url.has_value())
         {
            auto [[maybe_unused]] bytes_written = co_await io_pool.sndWriteFile(image_contents.file_path, image_contents.contents);
            SPDLOG_DEBUG("CtDatasetMgr::retrieveLabelImageAsync - saved {} bytes to '{}'", bytes_written, image_contents.file_path);
         }
         // just forward the data to the next sender
         co_return image_contents;
      };

      auto errorCallback = [this, result_callback](std::exception_ptr ep) noexcept
      {
         return safeErrorCallback(cpu_pool.get_scheduler(), result_callback, ep);
      };

      auto pipeline =
         just(wine_id)
         | continues_on(io_pool.get_scheduler())
         | let_value(getImageContents)
         | let_value(saveIfNeeded)
         | continues_on(cpu_pool.get_scheduler())
         | then(result_callback)
         | stopped_as_error(std::make_exception_ptr(Error{ constants::STATUS_DOWNLOAD_CANCELED, Error::Category::OperationCanceled }))
         | let_error(errorCallback)
         | stdexec::write_env(stdexec::prop(stdexec::get_stop_token, cancel_token));

      start_detached(move(pipeline));
   }


}   // namespace ctb
