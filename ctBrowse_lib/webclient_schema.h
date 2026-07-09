#pragma once

#include "ctb/ctb.h"
#include <glaze/json/raw_string.hpp>

#include <map>

namespace ctb::web
{

   namespace commands
   {
      inline constexpr const char* ATTACH_TARGET      = "Target.attachToTarget";
      inline constexpr const char* CLOSE_BROWSER      = "Browser.close";
      inline constexpr const char* CLOSE_TARGET       = "Target.closeTarget";
      inline constexpr const char* CREATE_TARGET      = "Target.createTarget";
      inline constexpr const char* ENABLE_PAGE_EVENTS = "Page.enable";
   }   // namespace commands

   namespace params
   {
      inline constexpr const char* ABOUT_BLANK  = "about:blank";
      inline constexpr const char* FLATTEN      = "flatten";
      inline constexpr const char* SESSION_ID   = "sessionId";
      inline constexpr const char* TARGET_ID    = "targetId";
      inline constexpr const char* TARGET_URL   = "url";
      inline constexpr const char* WS_DEBUG_URL = "webSocketDebuggerUrl";

   }   // namespace params


   using MaybeJson   = std::optional<glz::raw_json>;
   using JsonProp    = std::variant<int, std::string, bool>;
   using JsonPropMap = std::map<std::string, JsonProp>;


   /// @brief struct defining the message that is sent to the browser for commands
   struct BrowserCommand
   {
      std::string method{};
      NullableInt id{};
      MaybeString sessionId{};
      JsonPropMap params{};
   };


   /// @brief message that the browser returns to client.
   struct BrowserMessage
   {
      NullableInt id{};          // Present on Command Responses
      MaybeString method{};      // Present on Events
      MaybeString sessionId{};   // Present if using flat sessions

      // only one of these will be present
      MaybeJson result{};
      MaybeJson error{};
      MaybeJson params{};
   };


   struct CreateTargetResult
   {
      std::string targetId{};
   };


   struct AttachTargetResult
   {
      std::string sessionId{};
   };

}   // namespace ctb::web
