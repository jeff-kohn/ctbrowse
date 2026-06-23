#pragma once

#include "ctb/ctb.h"
#include "ctb/utility.h"
#include "HttpDownloader.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/awaitable.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/this_coro.hpp>

namespace ctb::tasks
{
   using asio::awaitable;
   using asio::stream_file;

   /// @brief awaitable task for saving a 
   awaitable<void> save_data_async(std::string filepath, std::string data)
   {
      auto executor = co_await asio::this_coro::executor;

      // Open file for asynchronous writing
      asio::stream_file file(executor, filepath, asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate);

      // Asynchronously write the complete buffer to disk.
      // Moving the data into the coroutine ensures its lifetime matches the async operation.
      co_await asio::async_write(file, asio::buffer(data), asio::use_awaitable);

      // File is automatically closed when `file` goes out of scope.
   }

   /// @brief Validates the response received from a cellartracker get request for a table download.
   /// @throw ctb::Error if the response object indicates any error (either network level or from CT)
   void validateDownloadTableResponse(const HttpDownloader::HttpResult response) noexcept(false)
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
   }

   
   /// @brief the result of a download will contain the requested data if successful, or an Error object if unsuccessful.
   using DownloadResult = std::expected<RawTableData, Error>;



   RawTableData getTableFromResponse(HttpDownloader::HttpResult response, TableId table_id) noexcept(false)
   {
      DownloadResult result{};
      try
      {
         validateDownloadTableResponse(response);
         std::string content_type_header{};
         if (auto it = response->response_headers.find(ctb::headers::CONTENT_TYPE_KEY); it != response->response_headers.end())
         {
            content_type_header = it->second;
         }
         // we got a table (or some sort of body), package the result.
         result = RawTableData{ .data        = std::move(response->response_body),
                                .table_id    = table,
                                .data_format = DataFormatId::csv,
                                .encoding    = getTextEncodingFromHeader(content_type_header).value_or(TextEncoding::ANSI) };
      }
      catch (ctb::Error& e)
      {
         result = std::unexpected{ std::move(e) };
      }

      // Now we can send our caller the table data (or Error object). Note we're still on asio thread here
      // so make sure no exceptions can escape
      try
      {
         callback(std::move(result));
      }
      catch (...)   // NOLINT
      {
         SPDLOG_DEBUG("Exception was thrown from downloadRawTableData callback: {}", packageError().formattedMessage());
      }
   };




}
