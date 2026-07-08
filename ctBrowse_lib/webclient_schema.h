#pragma once

#include "ctb/ctb.h"
#include <glaze/json/raw_string.hpp>

#include <map>

namespace ctb::webclient
{

   namespace commands
   {
      inline constexpr const char* CREATE_TARGET = "Target.createTarget";
      inline constexpr const char* CLOSE_TARGET  = "Target.closeTarget";
      inline constexpr const char* ATTACH_TARGET = "Target.attachToTarget";
   }   // namespace commands

   namespace params
   {
      inline constexpr const char* ABOUT_BLANK  = "about:blank";
      inline constexpr const char* SESSION_ID   = "sessionId";
      inline constexpr const char* TARGET_ID    = "targetId";
      inline constexpr const char* TARGET_URL   = "url";
      inline constexpr const char* WS_DEBUG_URL = "webSocketDebuggerUrl";

   }   // namespace params


   using MaybeJson      = std::optional<glz::raw_json>;
   using StringMap      = std::map<std::string, std::string>;
   using MaybeStringMap = std::optional<StringMap>;


   /// @brief struct defining the message that is sent to the webclient for commands
   struct BrowserCommand
   {
      std::string    method{};
      NullableInt    id{};
      MaybeString    sessionId{};
      MaybeStringMap params{};
   };


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

}   // namespace ctb::webclient
