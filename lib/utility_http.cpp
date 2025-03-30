#include "ctb/utility_http.h"

#include "external/HttpStatusCodes.h"

#include <cpr/curlholder.h>
#include <cpr/status_codes.h>
#include <curl/curl.h>
#include <curl/easy.h>

namespace ctb
{

   auto percentEncode(std::string_view text) noexcept -> std::string
   {
      // this just makes sure that curl's global init has been called, we don't actually need a handle
      cpr::CurlHolder holder;

      char* output = curl_easy_escape(nullptr, text.data(), static_cast<int>(text.length()));
      if (output)
      {
         std::string result{ output };
         curl_free(output);
         return result;
      }
      return std::string{ text };
   }


   auto percentDecode(std::string_view text) noexcept -> std::string
   {
      // this just makes sure that curl's global init has been called, we don't actually need a handle
      cpr::CurlHolder holder;

      char* output = curl_easy_escape(nullptr, text.data(), static_cast<int>(text.length()));
      if (output)
      {
         std::string result = output;
         curl_free(output);
         return result;
      }
      return std::string{ text };
   }


   auto validateResponse(cpr::Response& response) noexcept -> std::expected<bool, ctb::Error>
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


} // namespace ctb