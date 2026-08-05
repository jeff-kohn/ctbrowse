#pragma once

#include "ctb/ctb.h"
#include <glaze/json/raw_string.hpp>

#include <map>
#include <string_view>

namespace ctb
{

   namespace commands
   {
      inline constexpr const char* ATTACH_TARGET         = "Target.attachToTarget";
      inline constexpr const char* CLOSE_BROWSER         = "Browser.close";
      inline constexpr const char* CLOSE_TARGET          = "Target.closeTarget";
      inline constexpr const char* CREATE_TARGET         = "Target.createTarget";
      inline constexpr const char* ADD_NEW_DOC_SCRIPT    = "Page.addScriptToEvaluateOnNewDocument";
      inline constexpr const char* ENABLE_PAGE_EVENTS    = "Page.enable";
      inline constexpr const char* ENABLE_NETWORK_EVENTS = "Network.enable";
      inline constexpr const char* NAVIGATE_TO_URL       = "Page.navigate";
      inline constexpr const char* RUNTIME_EVAL          = "Runtime.evaluate";
      inline constexpr const char* SET_USER_AGENT        = "Network.setUserAgentOverride";
      inline constexpr const char* GET_RESOURCE_CONTENT  = "Page.getResourceContent";

   }   // namespace commands

   namespace params
   {
      inline constexpr const char* ABOUT_BLANK   = "about:blank";
      inline constexpr const char* EXPRESSION    = "expression";
      inline constexpr const char* ERROR_TEXT    = "errorText";
      inline constexpr const char* FLATTEN       = "flatten";
      inline constexpr const char* FRAME_ID      = "frameId";
      inline constexpr const char* RETURN_BY_VAL = "returnByValue";
      inline constexpr const char* SESSION_ID    = "sessionId";
      inline constexpr const char* TARGET_ID     = "targetId";
      inline constexpr const char* TARGET_URL    = "url";
      inline constexpr const char* WS_DEBUG_URL  = "webSocketDebuggerUrl";
      inline constexpr const char* SOURCE        = "source";
      inline constexpr const char* JS_ERROR_VAL  = "ERROR";

      inline constexpr const char* STEALTH_NAVIGATOR_NEW_DOC_SCRIPT =
         "Object.defineProperty(navigator, 'webdriver', {get: () => false}); window.chrome = { runtime: {} };"
         "Object.defineProperty(navigator, 'plugins', {get: () => [1, 2, 3]});"
         "Object.defineProperty(navigator, 'languages', {get: () => ['en-US', 'en']});"
         "Object.defineProperty(navigator, 'hardwareConcurrency', { get: () => 8 });"
         "Object.defineProperty(navigator, 'deviceMemory', { get : () => 16 });";

      inline constexpr const char* SET_USER_AGENT_PARAMS = R"({
          "userAgent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",
          "acceptLanguage": "en-US,en;q=0.9",
          "platform": "Win32",
          "userAgentMetadata": {
            "architecture": "x86",
            "bitness": "64",
            "mobile": false,
            "model": "",
            "platform": "Windows",
            "platformVersion": "10.0",
            "brands": [
              {"brand": "Not_A Brand", "version": "8"},
              {"brand": "Chromium", "version": "120"},
              {"brand": "Microsoft Edge", "version": "120"}
            ]
          }
      })";


      /// @brief Format string for JS expression to determine whether current browser session is logged into CT.
      ///        need to insert values to use for logged in, logged_out, and unkown
      inline constexpr std::string_view FMT_CHECK_LOGIN_STATUS_JS =
         R"((() => {{ 
            // Check for logged-in specific elements
            if (document.querySelector('.welcome_options')) {{
                let welcomeTag = document.querySelector('h1.welcome');
                let username = welcomeTag ? welcomeTag.innerText.replace('Welcome ', '') : 'User';
                return "{}:" + username;
            }}
            
            // Check for logged-out specific elements
            let spans = document.querySelectorAll('button span');
            let hasSignInBtn = Array.from(spans).some(span => span.innerText.trim() === 'Sign In');
            let hasPasswordField = document.querySelector('input[type="password"]');
            let isLoginPage = window.location.href.includes('password.asp') || window.location.href.includes('login');
            
            if (hasSignInBtn || hasPasswordField || isLoginPage) {{
                return "{}";
            }}
            
            return "ERROR: Could not determine logon status, page may not have finished loading.";
        }})())";

         
/* R"((() => {{ 
            let lightbox = document.getElementById('lightbox');
            if (lightbox && lightbox.getAttribute('data-username')) {{
                return "{}:" + lightbox.getAttribute('data-username');
            }}
            if (document.querySelector('input[type="password"]') || window.location.href.includes('login')) {{
                return "{}";
            }}
            return "{}";
        }})())";*/


      /// @brief Format string for JS expression to populate the login form inputs.
      inline constexpr std::string_view FMT_FILL_LOGIN_FORM_JS =
         R"((() => {{ 
            let userField = document.querySelector('input[name="szUser"], input[id="handle"]');
            let passField = document.querySelector('input[name="szPassword"], input[id="password"]');
            if (!userField || !passField) return 'ERROR: login inputs not found';
            userField.value = '{}';
            userField.dispatchEvent(new Event('input', {{ bubbles: true }}));
            passField.value = '{}';
            passField.dispatchEvent(new Event('input', {{ bubbles: true }}));
            return 'SUCCESS';
        }})())";

      /// @brief JavaScript expression to click the submit button
      inline constexpr std::string_view SUBMIT_LOGIN_FORM_EXPRESSION =
         R"((() => { 
            // Specifically target the sign in button by its exact ID
            let btn = document.getElementById('sign_in');
            if (btn) {
                btn.click();
                return 'SUCCESS';
            }
            // Fallback: If button is missing, find the specific login form and submit it programmatically
            let form = document.querySelector('form[name="login"]');
            if (form) {
                form.submit();
                return 'SUCCESS_FORM_SUBMIT';
            }
            return 'ERROR: submit button and form not found';
        })())";

   }   // namespace params

   namespace events
   {

      inline constexpr const char* PAGE_LOAD             = "Page.loadEventFired";
      inline constexpr const char* PAGE_DOM_LOADED       = "Page.domContentEventFired";
      inline constexpr const char* PAGE_FRAME_NAVIGATED  = "Page.frameNavigated";
      inline constexpr const char* NET_RESPONSE_RECEIVED = "Network.responseReceived";
   }   // namespace events


   using MaybeJson   = std::optional<glz::raw_json>;
   using JsonProp    = std::variant<int, std::string, bool, glz::raw_json>;
   using JsonPropMap = std::map<std::string, JsonProp>;


   /// @brief struct defining the message that is sent to the browser for commands
   struct BrowserCommand
   {
      NullableInt   id{};
      std::string   method{};
      MaybeString   sessionId{};
      glz::raw_json params{};
   };


   /// @brief message that the browser returns to client.
   ///
   /// Not all fields will be present in every message which is why they're all optional/nullable.
   /// If present, the result property could contain one of a number of JSON objects which can be parsed into
   /// result structs such as CreateTargetResult etc. Params will usually contain a dictionary of key-value
   /// pairs, while the error may contain error information for some commands
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


   /// @brief result of a Target.createTarget command
   struct CreateTargetResult
   {
      std::string targetId{};
   };

   /// @brief  result of a Target.attach command
   struct AttachTargetResult
   {
      std::string sessionId{};
   };

   /// @brief result of a Page.navigate command
   struct NavigateResult
   {
      std::string session_id{};
      std::string frame_id{};
      std::string url{};
   };


   /// @brief result of a Runtime.evaluate command
   struct RuntimeEvalResult
   {
      struct InnerResult
      {
         std::string type{};
         std::string value{};
      } result;
   };

   struct GetResourceResult
   {
      std::string content{};
      bool        base64Encoded{};
   };

}   // namespace ctb
