#pragma once

#include "ctb/ctb.h"
#include "ctb/CredentialWrapper.h"

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


   /// @brief Validates the supplied task result, and throws if validation fails
   /// 
   /// You can pass this function an l-value or r-value and it will do the right
   /// thing to "pass through" the response to the return value without modifying
   /// or copying it.
   /// 
   /// @return either a reference to or moved-to copy of response, depending 
   ///         on whether response is an l-value or r-value.
   /// @throws ctb::Error if validation fails
   /// 
   template<typename Resp> requires std::same_as<std::decay_t<Resp>, cpr::Response> 
   auto validateOrThrow(Resp&& response) noexcept(false) -> Resp
   {
      auto result = validateResponse(response);
      if (!result)
         throw result.error();

      return std::forward<Resp>(response);
   }


   /// @brief Retrieves view of HTTP response's content as a byte span along with its content-type
   /// 
   /// Note that both values are views into the  Response they  were generated 
   /// from. This function only accepts l-value reference to avoid returning 
   /// dangling views to temporaries
   /// 
   /// @return a pair containing a span<byte> for the data with a string_view 
   ///         for the content type. Both will be empty if the response doesn't
   ///         contain any data
   /// 
   auto getBytes(cpr::Response& response) -> std::pair<BufferSpan, std::string_view>;


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


   /// @brief result type for doCellarTrackerLogin()
   ///
   using CookieResult = std::expected<cpr::Cookies, ctb::Error>;

   /// @brief Create a logon session for interacting with the CellarTracker website
   /// 
   /// Connects to the CT website using the supplied credential and retrieves user Cookies
   /// for connection to and interacting with CT website.
   /// 
   /// @return the requested cookies if successful, a ctb::Error if unsuccessful.
   /// 
   auto doCellarTrackerLogin(CredentialWrapper::Credential& cred) -> cpr::Cookies;
}