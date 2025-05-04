#include "ctb/task/tasks.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"

#include <cpr/cpr.h>


namespace ctb::tasks
{
   using namespace std::literals;
   using cpr::Url;
   using std::exception;
   using std::expected;
   using std::stop_token;
   using std::string_view;
   using std::vector;
   using std::unexpected;


   auto runLoadFileTask(fs::path file, stop_token token) noexcept(false) -> FetchFileTask::ReturnType
   {
      // there was originally more to these functions when implementing a coroutine for libcoro, but
      // when that approach was abandoned for std::async() there wasn't much left. I decided to keep the 
      // encapsulation because at some point I'll probably want to move to asio/cobalt or P2300/?? or ??/??...
      checkStopToken(token);
      return readBinaryFile(file);
   }


   //auto runHttpGetTask(std::string url, stop_token token) noexcept(false) -> HttpRequestTask::ReturnType
   //{
   //   checkStopToken(token);
   //   return validateOrThrow(cpr::Get(Url{ url }));
   //}



   auto runLabelDownloadTask(uint64_t wine_id, std::stop_token token) noexcept(false)-> FetchFileTask::ReturnType
   {
      checkStopToken(token);

      // First, execute task to get the initial HTTP request for the wine page.
      // no need to validate response since it's direct function call and runHttpGetTask
      // already validates it. That might change if this moves to coroutine impl in future
      cpr::Redirect redir{1, true, false, cpr::PostRedirectFlags::POST_301 | cpr::PostRedirectFlags::POST_302};
      auto response = runHttpGetTask(getWineDetailsUrl(wine_id), token, getPageRequestHeaders());
      
      SPDLOG_DEBUG("runLabelDownloadTask({}) successfully got wine details html.", wine_id);

      // parse the HTML to get the URL for the label image.
      checkStopToken(token);
      auto img_url = parseLabelUrlFromHtml(response.text);
      if (img_url.empty())
      {
         throw Error{ constants::ERROR_STR_LABEL_URL_NOT_FOUND };
      }

      SPDLOG_DEBUG("runLabelDownloadTask({}) parsed image url of {}.", wine_id, img_url);

      // now execute task to download the label image from the web
      checkStopToken(token);
      response = runHttpGetTask(img_url, token, getImageRequestHeaders());
      auto [buf, type] = getBytes(response);
      
      SPDLOG_DEBUG("runLabelDownloadTask({}) downloaded {} bytes with content type {}.", wine_id, buf.size(), type);

      return Buffer{ std::from_range, buf };
   }



   //auto runCellarTrackerLogin(CredentialWrapper&& cred, std::stop_token token) noexcept(false) -> LoginTask::ReturnType
   //{
   //   using std::string;

   //   checkStopToken(token);

   //   cpr::Url url{ constants::URL_CT_LOGIN_FORM };
   //   cpr::Payload form_data{
   //      { string{ constants::HTTP_PARAM_KEY_REFERRER   }, string{ constants::HTTP_PARAM_VAL_REFERRER   } },
   //      { string{ constants::HTTP_PARAM_KEY_USER       }, string{ cred.username()                      } },
   //      { string{ constants::HTTP_PARAM_KEY_PASSWORD   }, string{ cred.password()                      } },
   //      { string{ constants::HTTP_PARAM_KEY_USE_COOKIE }, string{ constants::HTTP_PARAM_VAL_USE_COOKIE } }
   //   };

   //   using namespace constants;
   //   cpr::Header header{ 
   //      { HTTP_ACCEPT_KEY,          HTTP_ACCEPT_HTML            },
   //      { HTTP_ACCEPT_LANG_KEY,     HTTP_ACCEPT_LANG_VAL        },
   //      { HTTP_PRIORITY_KEY,        HTTP_PRIORITY_VAL           },
   //      { HTTP_REFERRER_KEY,        "https://www.cellartracker.com/password.asp?Referrer=%2Fdefault%2Easp%3F"},
   //      { HTTP_SEC_UA_KEY,          HTTP_SEC_UA_VAL             },
   //      { HTTP_SEC_UA_PLATFORM_KEY, HTTP_SEC_UA_PLATFORM_VAL    },
   //      { HTTP_SEC_UA_MOBILE_KEY,   HTTP_SEC_UA_MOBILE_VAL      },
   //      { HTTP_SEC_FETCH_DEST_KEY,  HTTP_SEC_FETCH_DEST_DOC     },
   //      { HTTP_SEC_FETCH_MODE_KEY,  HTTP_SEC_FETCH_MODE_NAV     },
   //      { HTTP_SEC_FETCH_SITE_KEY,  HTTP_SEC_FETCH_SITE_SAME    },
   //      { HTTP_SEC_FETCH_USER_KEY,  HTTP_SEC_FETCH_USER_VAL     },
   //      { "sec-fetch-storage-access",          "none"},
   //      { HTTP_USERAGENT_KEY,       HTTP_USERAGENT_VAL          }
   //   };

   //   auto response = validateOrThrow(cpr::Post(url, form_data, header));
   //   return response.cookies;     
   //}

} // namespace ctb::tasks

 