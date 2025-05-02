/*********************************************************************
 * @file       table_download.cpp
 *
 * @brief      implements download of CT tables from CellarTracker.com
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/table_download.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"
#include "external/HttpStatusCodes.h"

#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cpr/status_codes.h>

namespace ctb
{
   /// @brief  returns true if the request returned a valid response, or an Error if it didn't
   ///
   std::expected<bool, ctb::Error> validateCtRequest(cpr::Response& response)
   {
      using namespace magic_enum;

      // unexpected return value, will be populated below if request failed.
      Error error{}; 

      if (cpr::status::is_success(response.status_code))
      {
         // our request was successful, but we need to check if the response contains
         // a file or an error message since CT returns an HTML <body> for some errors
         if (response.text == constants::ERR_STR_INVALID_CELLARTRACKER_LOGON)
         {
            error.error_code = static_cast<int64_t>(HttpStatus::Code::Unauthorized);
            error.error_message = constants::ERROR_STR_AUTHENTICATION_FAILED;
            error.category = Error::Category::HttpStatus;
         }
         else
            return true; // we actually got a file, so return success
      }
      else if (response.error.code != cpr::ErrorCode::OK)
      {
         error.error_code = static_cast<int64_t>(response.error.code);
         error.error_message = ctb::format(constants::FMT_ERROR_CURL_ERROR, error.error_code);

         // use a separate category for cancellation, so the caller can distinguish and avoid showing unnecessary error messages
         error.category = error.error_code == enum_index(cpr::ErrorCode::ABORTED_BY_CALLBACK) ? Error::Category::OperationCanceled
                                                                                              : Error::Category::CurlError;
      }
      else {
         error.error_code = static_cast<int64_t>(response.status_code);
         error.error_message = ctb::format(constants::FMT_ERROR_HTTP_STATUS_CODE, error.error_code);
         error.category = Error::Category::HttpStatus;
      }

      return std::unexpected{ error };
   }

} // anon namespace


namespace ctb
{

   [[nodiscard]] DownloadResult downloadRawTableData(const CredentialWrapper& cred, TableId table, DataFormatId format, ProgressCallback* callback )
   {
      auto table_name = magic_enum::enum_name(table);
      auto data_format = magic_enum::enum_name(format);

      cpr::Url url{ ctb::format(constants::FMT_URL_CT_TABLE,
                                percentEncode(cred.username()),
                                percentEncode(cred.password()),
                                data_format, table_name)
      };

      auto response = callback ? cpr::Get(url, *callback)
                               : cpr::Get(url);

      // check the response for success, bail out if we got an error 
      auto request_result = validateResponse(response);
      if (!request_result.has_value())
      {
         return std::unexpected{ request_result.error() };
      }
      RawTableData table_data{ std::move(response.text), table, format };

      // The returned data is encoded as ISO 8859-1 Latin 1, we need to convert
      // it to UTF-8 before returning it. If the conversion fails, just return the
      // original encoding as fallback.
      auto utf_text = toUTF8(table_data.data);
      if (utf_text)
      {
         table_data.data.swap(*utf_text);
      }

      return table_data;
   }


} // namespace ctb