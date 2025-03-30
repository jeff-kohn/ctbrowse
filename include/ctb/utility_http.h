#pragma once

#include "ctb/ctb.h"

#include <cpr/response.h>
#include <expected>

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
   /// calls to this function in a conditional to evaluate to success/failure if you don't
   /// care about retrieving the exact exception details.
   /// 
   auto validateResult(cpr::Response& response) noexcept -> std::expected<bool, ctb::Error>;
}