/*********************************************************************
 * @file       CellarTrackerDownload.cpp
 *
 * @brief      Implementation for the class CredentialWrapper
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "cts/CellarTrackerDownload.h"
#include "cts/constants.h"
#include "cts/HttpStatusCodes.h"
#include "cts/winapi_util.h"

#include "cpr/cpr.h"
#include "cpr/response.h"
#include "cpr/status_codes.h"
#include "magic_enum/magic_enum.hpp"

#include <format>

namespace cts::data
{
   namespace
   {
      using namespace magic_enum;

      /// @brief  returns true if the request returned a valid response, or an Error if it didn't
      std::expected<bool, Error> validateResult(cpr::Response& response)
      {
         Error error{}; // return value, will be populated below if request failed.

         if (cpr::status::is_success(response.status_code))
         {
            // our request was successful, but we need to check if the response contains
            // a file or an error message since CT returns an HTML <body> for some errors
            if (response.text == constants::ERR_INVALID_CELLARTRACKER_LOGON)
            {
               error.error_code = static_cast<int64_t>(HttpStatus::Code::Unauthorized);
               error.error_message = constants::ERROR_AUTHENTICATION_FAILED;
               error.category = Error::Category::HttpStatus;
            }
            else
               return true; // we actually got a file, so return success
         }
         else if (response.error.code != cpr::ErrorCode::OK)
         {
            error.error_code = static_cast<int64_t>(response.error.code);
            error.error_message = std::format(constants::FMT_ERROR_CURL_ERROR, error.error_code);

            // use a separate category for cancellation, so the caller can distinguish and avoid showing unnecessary error messages
            error.category = error.error_code == enum_index(cpr::ErrorCode::ABORTED_BY_CALLBACK) ? Error::Category::OperationCanceled
                                                                                                 : Error::Category::CurlError;
         }
         else {
            error.error_code = static_cast<int64_t>(response.status_code);
            error.error_message = std::format(constants::FMT_ERROR_HTTP_STATUS_CODE, error.error_code);
            error.category = Error::Category::HttpStatus;
         }

         return std::unexpected{ error };
      }

   } // anon namespace


   CellarTrackerDownload::DownloadResult
      CellarTrackerDownload::getTableData(const CredentialWrapper::Credential& cred,
                                          TableId table,
                                          DataFormatId format,
                                          ProgressCallback* callback)
   {
      using cpr::cpr_off_t;
      auto table_name = magic_enum::enum_name(table);
      auto data_format = magic_enum::enum_name(format);
      cpr::Header header{ {constants::HTTP_HEADER_XCLIENT, constants::HTTP_HEADER_XCLIENT_VALUE} };


      cpr::Url url{ std::format(constants::FMT_HTTP_CELLARTRACKER_QUERY_URL,
                                 util::percentEncode(std::string(cred.username)),
                                 util::percentEncode(std::string(cred.password)),
                                 data_format, table_name)
      };

      auto response = callback ? cpr::Get(url, header, *callback)
                               : cpr::Get(url, header);

      // check the response for success, bail out if we got an error 
      auto request_result = validateResult(response);
      if (!request_result.has_value())
         return std::unexpected{ request_result.error() };

      TableData table_data{ std::move(response.text), table, format };

      // The returned data is encoded as ISO 8859-1 Latin 1, we need to convert
      // it to UTF-8 before returning it. If the conversion fails, just return the
      // original encoding as fallback.
      auto utf_text = util::toUTF8(table_data.data);
      if (!utf_text.empty())
         table_data.data.swap(utf_text);

      return table_data;
   }

}  // namespace cts::data
