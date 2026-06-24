#pragma once

#include "ctb/HttpDownloader.h"
#include "ctb/ctb.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"

#include <asio/buffer.hpp>
#include <asio/write.hpp>
#include <exec/asio/asio_thread_pool.hpp>
#include <exec/asio/use_sender.hpp>
#include <exec/start_detached.hpp>
#include <exec/static_thread_pool.hpp>
#include <stdexec/execution.hpp>

namespace ctb::tasks
{

   // keep namespace pollution out of our expressions, especially since stdexec will probably become std::exec
   using exec::start_detached;
   using exec::asio::use_sender;
   using stdexec::continues_on;
   using stdexec::just;
   using stdexec::let_error;
   using stdexec::let_value;
   using stdexec::then;
   using stdexec::upon_error;


   /// @brief checks an CellarTracker HTTP response for errors and throws them as Error exceptions
   /// @return the response object from the HttpResult if validation passed
   /// @throw ctb::Error if validation fails
   auto validateHttpResponse(const HttpDownloader::HttpResult response) noexcept(false) -> HttpDownloader::HttpResult::value_type
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



   template<typename SchedulerT, typename CallbackT>
   auto safeSuccessCallback(SchedulerT scheduler, CallbackT&& callback) noexcept
   {
      return just(std::move(ep))
           | continues_on(scheduler)
           | then(
                [cb_func = std::forward<CallbackT>(callback)](std::exception_ptr ep) mutable noexcept
                {
                   auto error = packageError(ep);
                   SPDLOG_DEBUG(error.formattedMessage());
                   cb_func(std::unexpected{ std::move(error) });
                })
           | upon_error(
                []([[maybe_unused]] std::exception_ptr ep) noexcept
                {
                   try
                   {
                      SPDLOG_DEBUG("safeErrorCallback caught a leaKed exception from callback invocation: {}",
                                   packageError(ep).formattedMessage());
                   }
                   catch (...)
                   {}   // NOLING
                });
   }



   /// @brief Sender that runs on the the specified scheduler and calls the error callback without allowing exceptions to escape.
   /// @return the sender that can be assigned to a receiver for async execution.
   template<typename SchedulerT, typename CallbackT>
   auto safeErrorCallback(SchedulerT scheduler, CallbackT&& callback, std::exception_ptr ep) noexcept
   {
      return just(std::move(ep))
           | continues_on(scheduler)
           | then(
                [cb_func = std::forward<CallbackT>(callback)](std::exception_ptr ep) mutable noexcept
                {
                   auto error = packageError(ep);
                   SPDLOG_DEBUG(error.formattedMessage());
                   cb_func(std::unexpected{ std::move(error) });
                })
           | upon_error(
                []([[maybe_unused]] std::exception_ptr ep) noexcept
                {
                   try
                   {
                      SPDLOG_DEBUG("safeErrorCallback caught a leaKed exception from callback invocation: {}",
                                   packageError(ep).formattedMessage());
                   }
                   catch (...)
                   {}   // NOLING
                });
   }


}   // namespace ctb::tasks
