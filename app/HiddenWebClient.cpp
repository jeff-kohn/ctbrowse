#include "App.h"
#include "HiddenWebClient.h"


namespace ctb::app
{
   static inline constexpr auto* GET_LABEL_SCRIPT = "document.querySelector('#label_photo img').src;";


   auto HiddenWebClient::create() ->std::expected<WebClientPtr, ctb::Error>
   {
      if (!wxWebView::IsBackendAvailable(wxWebViewBackendEdge))
      {
         return std::unexpected{ ctb::Error{ Error::Category::GenericError, "WebView backend not available, online label image will be disabled"} };
      }

      WebClientPtr wnd{ new HiddenWebClient{} };
      wnd->createWindow();
      return wnd;
   }


   auto HiddenWebClient::requestPage(std::string url, PageLoadedCallback callback) -> bool
   {
      if (m_busy_flag)
         return false;

      m_busy_flag = true;
      m_requests.try_emplace(url, std::move(callback));
      m_webview->LoadURL(wxFromSV(url));

      return true;
   }


   void HiddenWebClient::createWindow()
   {
      if (!Create(nullptr, wxID_ANY, "Hidden WebClient"))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      m_webview = wxWebView::New(this, wxID_ANY);
      assert(m_webview);

      m_webview->Bind(wxEVT_WEBVIEW_LOADED, &HiddenWebClient::onPageLoaded, this);
      m_webview->Bind(wxEVT_WEBVIEW_ERROR, &HiddenWebClient::onError, this);
      m_webview->Bind(wxEVT_WEBVIEW_SCRIPT_RESULT, &HiddenWebClient::onScriptResult, this);
      m_webview->Bind(wxEVT_WEBVIEW_NAVIGATING, &HiddenWebClient::onNavigating, this);
      m_webview->Bind(wxEVT_WEBVIEW_NAVIGATED, &HiddenWebClient::onNavigated, this);

      Show(false);
   }


   void HiddenWebClient::onPageLoaded(wxWebViewEvent& event)
   {
      log::info("HiddenWebClient::onPageLoaded - URL: '{}'", wxViewString(event.GetURL()));

      m_busy_flag = false;
      auto url = event.GetURL().utf8_string();
      if (auto it = m_requests.find(url); it != m_requests.end())
      {
         it->second(m_webview->GetPageSource().utf8_string());
      }
      else if (url != constants::URL_ABOUT_BLANK)
      {
         assert(false);
         log::error("HiddenWebClient::onPageLoaded called for URL with no callback/request object!");
      }
   }

   void HiddenWebClient::onNavigating(wxWebViewEvent& event)
   {
      if (!m_busy_flag)
      {
         log::info("wxWebView::OnNavigating: Veto navigation for '{}', page already loaded", wxViewString(event.GetURL()));
         event.Veto();
      }
      else
         log::info("wxWebView::OnNavigating: URL: '{}', Target: '{}'", wxViewString(event.GetURL()), wxViewString(event.GetTarget()));
   }

   void HiddenWebClient::onNavigated(wxWebViewEvent& event)
   {
      log::info("wxWebView::OnNavigated: URL: '{}', Target: '{}'", wxViewString(event.GetURL()), wxViewString(event.GetTarget()));
   }

   void HiddenWebClient::onError(wxWebViewEvent& event)
   {
      log::info("HiddenWebClient::onError - URL: '{}', Target: '{}', Is error: {}, Is main frame: {}, Navigation action: {}, Message: '{}'",
         wxViewString(event.GetURL()),
         wxViewString(event.GetTarget()),
         event.IsError(),
         event.IsTargetMainFrame(),
         static_cast<int>(event.GetNavigationAction()),
         wxViewString(event.GetString()));

      if (auto it = m_requests.find(event.GetURL().utf8_string()); it != m_requests.end())
      {
         it->second(
            std::unexpected
            {
               ctb::Error{ Error::Category::GenericError, "WebClient backend encoutered error attempting to load page {}. {}", it->first, wxViewString(event.GetString()) }
            });
      }
      else {
         assert(false);
         log::error("HiddenWebClient::onError called for URL with no callback/request object!");
      }
   }


   void HiddenWebClient::onScriptResult(wxWebViewEvent& event)
   {
      log::info("HiddenWebClient::onScriptResult - URL: '{}', Target: '{}', Is error: {}, Is main frame: {}, Navigation action: {}, Message: '{}'",
         wxViewString(event.GetURL()),
         wxViewString(event.GetTarget()),
         event.IsError(),
         event.IsTargetMainFrame(),
         static_cast<int>(event.GetNavigationAction()),
         wxViewString(event.GetString()));
   }   
}

