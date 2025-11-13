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

#include <cpr/response.h>
#include <cpr/cprtypes.h>
#include <fmt/chrono.h>

#include <expected>
#include <string>
#include <string_view>

namespace ctb
{
   namespace headers
   {
      inline constexpr const char* USERAGENT_KEY          = "user-agent";
      inline constexpr const char* USERAGENT_VAL          = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36";
      inline constexpr const char* CONTENT_TYPE_KEY       = "Content-Type";
      inline constexpr const char* CONTENT_TYPE_JPEG      = "image/jpeg";
      inline constexpr const char* CONTENT_TYPE_UTF8      = "text/plain;Charset=UTF-8";


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


   }

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
   inline auto getPageRequestHeaders(const std::string& referer = constants::URL_CT_DOT_COM) noexcept -> cpr::Header
   {
      using namespace headers;

      return cpr::Header{ 
         { ACCEPT_KEY,          ACCEPT_HTML            },
         { ACCEPT_LANG_KEY,     ACCEPT_LANG_VAL        },
         { CACHE_CONTROL_KEY,   VALUE_NONE             },
         { PRAGMA_KEY,          VALUE_NONE             },
         { PRIORITY_KEY,        PRIORITY_VAL           },
         { REFERRER,            referer                },
         { SEC_UA_KEY,          SEC_UA_VAL             },
         { SEC_UA_PLATFORM_KEY, SEC_UA_PLATFORM_VAL    },
         { SEC_UA_MOBILE_KEY,   SEC_UA_MOBILE_VAL      },
         { SEC_FETCH_DEST_KEY,  FETCH_DEST_DOC         },
         { SEC_FETCH_MODE_KEY,  FETCH_MODE_NAV         },
         { SEC_FETCH_SITE_KEY,  SAME_ORIGIN            },
         { SEC_FETCH_USER_KEY,  SEC_FETCH_USER_VAL     },
         { USERAGENT_KEY,       USERAGENT_VAL          }
      };
   }

   /// @brief get default headers to use for HTTP requests to CellarTracker.com
   /// 
   inline auto getImageRequestHeaders(const std::string& referer = constants::URL_CT_DOT_COM) noexcept -> cpr::Header
   {
      using namespace headers;

      return cpr::Header{ 
         { ACCEPT_KEY,          ACCEPT_IMG             },
         { ACCEPT_LANG_KEY,     ACCEPT_LANG_VAL        },
         { CACHE_CONTROL_KEY,   VALUE_NONE             },
         { PRAGMA_KEY,          VALUE_NONE             },
         { PRIORITY_KEY,        PRIORITY_VAL           },
         { REFERRER,            referer                },
         { SEC_UA_KEY,          SEC_UA_VAL             },
         { SEC_UA_PLATFORM_KEY, SEC_UA_PLATFORM_VAL    },
         { SEC_UA_MOBILE_KEY,   SEC_UA_MOBILE_VAL      },
         { SEC_FETCH_DEST_KEY,  FETCH_DEST_IMG         },
         { SEC_FETCH_MODE_KEY,  FETCH_MODE_NOCORS      },
         { SEC_FETCH_SITE_KEY,  CROSS_SITE             },
         { SEC_FETCH_USER_KEY,  SEC_FETCH_USER_VAL     },
         { USERAGENT_KEY,       USERAGENT_VAL          }
      };
   }


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
      auto wine_param = fromUTF8(wine, CP_WINDOWS_1252).value_or(wine);
      return ctb::format(constants::FMT_URL_CT_VINTAGES, percentEncode(wine_param));
   }

   inline auto getDrinkWindowUrl(uint64_t wine_id) -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_DRINK_WINDOW, wine_id);
   }

   /// @brief get the CT URL for accepting a pending delivery
   inline auto getAcceptPendingUrl(uint64_t wine_id, std::string_view purch_id, const std::chrono::year_month_day& delivery_date) noexcept -> std::string
   {
      return ctb::format(constants::FMT_URL_CT_ACCEPT_PENDING, wine_id, purch_id, delivery_date);
   }

   /// @brief get the CT URL for editing a pending order
   inline auto getEditPendingUrl(uint64_t wine_id, std::string_view purchase_id) noexcept -> std::string
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


}