/*********************************************************************
 * @file       table_download.cpp
 *
 * @brief      implements download of CT tables from CellarTracker.com
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/table_download.h"
#include "TableDownloader.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"
#include "external/HttpStatusCodes.h"

#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cpr/status_codes.h>

namespace ctb
{
   namespace
   {

      auto getDownloader() -> TableDownloader&
      {
         static TableDownloader downloader;
         return downloader;
      }

   }   // namespace


   [[nodiscard]] auto downloadRawTableData(const CredentialWrapper& cred,
                                           TableId                  table,
                                           DataFormatId             format,
                                           ProgressCallback*        callback,
                                           bool                     convert_to_utf,
                                           uint32_t                 table_code_page) -> DownloadResult
   {
      auto table_name  = enum_to_string(table);
      auto data_format = enum_to_string(format);

      cpr::Url url{
         ctb::format(constants::FMT_URL_CT_TABLE, percentEncode(cred.username()), percentEncode(cred.password()), data_format, table_name)
      };

      auto response = callback ? cpr::Get(url, *callback) : cpr::Get(url);

      // check the response for success, bail out if we got an error
      auto request_result = validateResponse(response);
      if (!request_result.has_value())
      {
         return std::unexpected{ request_result.error() };
      }
      RawTableData table_data{ std::move(response.text), table, format };

      if (convert_to_utf)
      {
         // The returned data is encoded as Window-1252, we need to convert
         // it to UTF-8 before returning it. If the conversion fails, just return the
         // original encoding as fallback.
         if (auto utf_text = toUTF8(table_data.data, table_code_page))
         {
            table_data.data.swap(*utf_text);
         }
      }
      return table_data;
   }


   /// @brief Retrieve a data table from CT website asynchronously
   ///
   /// @param cred     - the username/password to use for the download
   /// @param table    - the table to retrieve
   /// @param callback - callback function to receive the result.
   /// @param format   - the data format to return
   ///
   void downloadTableAsync(const CredentialWrapper& cred, ResultCallback callback, TableId table, DataFormatId format)
   {
      // this is the lambda that will run on asio thread for the http client callback when the request is finished executing
      auto process_result = [callback = std::move(callback), table, format](TableDownloader::HttpResult response) mutable
      {
         DownloadResult result{};
         try
         {
            if (!response)
            {
               auto&& error = response.error();
               throw ctb::Error{ error.value(), error.message(), Error::Category::HttpStatus };
            }

            if (response)
            {
               if (HttpStatus::isSuccessful(response->status_code))
               {
                  // our request was successful, but we need to check if the response contains
                  // a file or an error message since CT returns an HTML <body> for auth errors
                  if (response->response_body.starts_with(constants::ERR_STR_INVALID_CELLARTRACKER_LOGON))
                  {
                     throw ctb::Error{
                        HttpStatus::toInt(HttpStatus::Code::Unauthorized),
                        constants::ERROR_STR_AUTHENTICATION_FAILED,
                        Error::Category::HttpStatus,
                     };
                  }
                  else
                  {
                     // we got a table (or some sort of body), package the result.
                     result = RawTableData{ .data = std::move(response->response_body), .table_id = table, .data_format = format };
                  }
               }
               else
               {
                  // we successfully received a response, but result code didn't indicate success.
                  auto status_msg = HttpStatus::reasonPhrase(response->status_code);

                  SPDLOG_DEBUG("downloadRawTableDataCallback received unexpected response: {} - {}. Body starts with {}",
                               response->status_code,
                               status_msg,
                               response->response_body.substr(0, 128));   // NOLINT

                  throw ctb::Error{ response->status_code, status_msg, Error::Category::HttpStatus };
               }
            }
            else
            {
               auto&& error = response.error();
               throw ctb::Error{ error.value(), Error::Category::HttpStatus, error.message() };
            }
         }
         catch (ctb::Error& e)
         {
            result = std::unexpected{ std::move(e) };
         }

         // Now we can send our caller their result. note we're still on asio thread here
         // so make sure no exceptions can escape
         try
         {
            callback(std::move(result));
         }
         catch (...)
         {
            SPDLOG_DEBUG("Exception was thrown from downloadRawTableData callback: {}", packageError().formattedMessage());
         }
      };

      // this is the entry point from caller thread, which creates the async job and immediately returns.
      getDownloader().downloadTable(cred, table, std::move(process_result));
   }


}   // namespace ctb
