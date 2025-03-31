#pragma once

#include "ctb/ctb.h"

#include <cpr/response.h>
#include <cpr/cprtypes.h>
#include <expected>
#include <string>
#include <string_view>

namespace ctb
{

   /// @brief  percent-encode a string to make it compatible with HTTP requests
   /// @return the encoded string, or a copy of the original text if the encoding failed
   /// 
   auto percentEncode(std::string_view text) noexcept -> std::string;


   /// @brief decode a percent-encoded string
   /// @return the decoded string, or a copy of the original text if the decoding failed
   /// 
   auto percentDecode(std::string_view text) noexcept -> std::string;


   /// @brief  returns true if the request returned a valid response, or an Error if it didn't
   ///
   /// the bool will always be true, ctb::Error will be returned otherwise. So you can use
   /// operator bool() in a conditional to evaluate to success/failure if you don't
   /// care about retrieving the exact exception details.
   /// 
   auto validateResponse(const cpr::Response& response) noexcept -> std::expected<bool, ctb::Error>;


   /// @brief parses an HTML fragment looking for the element containing the label_photo URL
   ///
   /// @return the requested URL if found, empty string otherwise.
   /// 
   auto parseLabelUrlFromHtml(const std::string& html) -> std::string;


   /// @brief get default headers to use for HTTP requests to CellarTracker.com
   /// 
   inline auto getDefaultHeaders() noexcept -> cpr::Header
   {
      return cpr::Header{ { constants::HTTP_USER_AGENT_NAME, constants::HTTP_USER_AGENT_VALUE } };
   }


   /// @brief get the URL for a Wine given it's iWineID
   ///
   inline auto getWineDetailsUrl(uint64_t wine_id) noexcept -> std::string
   {
      return ctb::format(constants::FMT_HTTP_CT_WINE_URL, wine_id);
   }
}