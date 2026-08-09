/*******************************************************************
* @file utility_http.h
*
* @brief Header file for some http-related helper functions/constants
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/utility.h"

#include <external/HttpStatusCodes.h>
#include <fmt/chrono.h>

#include <expected>
#include <string>
#include <string_view>


namespace ctb
{
   // clang-format off
   namespace headers
   {
      inline constexpr const char* USERAGENT_KEY          = "user-agent";
      inline constexpr const char* USERAGENT_VAL          = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36";
      inline constexpr const char* CONTENT_TYPE_KEY       = "content-type";
      inline constexpr const char* CONTENT_TYPE_JPEG      = "image/jpeg";
      inline constexpr const char* CONTENT_TYPE_UTF8      = "text/plain;charset=utf-8";


      inline constexpr const char* ACCEPT_KEY             = "accept";
      inline constexpr const char* ACCEPT_HTML            = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7";
      inline constexpr const char* ACCEPT_IMG             = "image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8";

      inline constexpr const char* ACCEPT_LANG_KEY        = "accept-language";
      inline constexpr const char* ACCEPT_LANG_VAL        = "en-US,en;q=0.9";

      inline constexpr const char* CACHE_CONTROL_KEY      = "cache-control";
      inline constexpr const char* NO_CACHE               = "no-cache";

      inline constexpr const char* PRAGMA_KEY             = "NO-CA";

      inline constexpr const char* PRIORITY_KEY           = "priority";
      inline constexpr const char* PRIORITY_VAL           = "u=0";

      inline constexpr const char* REFERRER               = "referer";

      inline constexpr const char* SEC_UA_KEY             = "sec-ch-ua";
      inline constexpr const char* SEC_UA_VAL             = R"("Chromium";v="136", "Brave";v="136", "Not.A/Brand";v="99")";

      inline constexpr const char* SEC_UA_PLATFORM_KEY    = "sec-ch-ua-platform";
      inline constexpr const char* SEC_UA_PLATFORM_VAL    = "?0";

      inline constexpr const char* SEC_UA_MOBILE_KEY      = "sec-ch-ua-mobile";
      inline constexpr const char* SEC_UA_MOBILE_VAL      = R"("Windows")";

      inline constexpr const char* SEC_FETCH_DEST_KEY     = "sec-fetch-dest";
      inline constexpr const char* FETCH_DEST_DOC         = "document";
      inline constexpr const char* FETCH_DEST_IMG         = "image";

      inline constexpr const char* SEC_FETCH_MODE_KEY     = "sec-fetch-mode";
      inline constexpr const char* FETCH_MODE_NAV         = "navigate";
      inline constexpr const char* FETCH_MODE_ORIGIN      = "same-origin";
      inline constexpr const char* FETCH_MODE_NOCORS      = "no-cors";

      inline constexpr const char* SEC_FETCH_SITE_KEY     = "sec-fetch-site";
      inline constexpr const char* SAME_ORIGIN            = "same-origin";
      inline constexpr const char* CROSS_SITE             = "cross-site";

      inline constexpr const char* SEC_FETCH_STORAGE      = "sec-fetch-storage-access";
      inline constexpr const char* VALUE_NONE             = "none";


      inline constexpr const char* SEC_FETCH_USER_KEY     = "sec-fetch-user";
      inline constexpr const char* SEC_FETCH_USER_VAL     = "?1";

      inline constexpr const char* ELEM_LABEL_PHOTO       = "label_photo";
      inline constexpr const char* ATTR_SRC               = "src";

   } // namespace headers
   // clang-format on


   /// @brief  percent-encode a string to make it compatible with HTTP requests
   /// @return the encoded string, or a copy of the original text if the encoding failed
   ///
   auto percentEncode(std::string_view text) noexcept -> std::string;


   /// @brief decode a percent-encoded string
   /// @return the decoded string, or a copy of the original text if the decoding failed
   ///
   auto percentDecode(std::string_view text) noexcept -> std::string;


   /// @brief get the CT URL for a Wine given it's iWineID
   ///
   /// Works with both string and numeric form of wine ID
   inline auto getWineDetailsUrl(uint64_t wine_id) noexcept -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_WINE_DETAILS, wine_id);
   }


   /// @brief get the CT URL for a Wine given it's iWineID
   inline auto getWineVintagesUrl(const std::string& wine) noexcept -> std::string
   {
      // CT can't handle UTF-8 strings, but browser assumes query parameters in URL's are UTF-8,
      // so convert to code page CT expects and then url-encode it so it doesn't get mangled
      auto wine_param = fromUTF8(wine, TextEncoding::WINDOWS_1252).value_or(wine);
      return ctb::format(constants::FMT_URL_CT_VINTAGES, percentEncode(wine_param));
   }


   inline auto getDrinkWindowUrl(uint64_t wine_id) -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_DRINK_WINDOW, wine_id);
   }


   /// @brief get the CT URL for accepting a pending delivery
   inline auto getAcceptPendingUrl(uint64_t wine_id, uint64_t purch_id, const std::chrono::year_month_day& delivery_date) noexcept -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_ACCEPT_PENDING, wine_id, purch_id, delivery_date);
   }


   /// @brief get the CT URL for editing a pending order
   inline auto getEditPendingUrl(uint64_t wine_id, uint64_t purchase_id) noexcept -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_EDIT_ORDER, wine_id, purchase_id);
   }


   inline auto getDrinkRemoveUrl(uint64_t wine_id) -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_DRINK_REMOVE, wine_id);
   }


   inline auto getAddToCellarUrl(uint64_t wine_id) -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_ADD_TO_CELLAR, wine_id);
   }


   inline auto getAddTastingNoteUrl(uint64_t wine_id) -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_ADD_TASTING_NOTE, wine_id);
   }


   auto getTextEncodingFromHeader(std::string content_type_header) -> std::optional<TextEncoding>;


}   // namespace ctb
