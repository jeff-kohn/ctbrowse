#pragma once

#include <wx/frame.h>
#include <wx/webview.h>
#include <wx/windowptr.h>

#include <expected>
#include <map>



namespace ctb::app
{
   class HiddenWebClient final : public wxFrame
   {
   public:
      using PageLoadedResult   = std::expected<std::string, ctb::Error>;
      using PageLoadedCallback = std::function<void(PageLoadedResult)>;

      // static factory method. Since this is a top-level window with no parent, we return a ref-counted
      // ptr that will cleanly destroy the window when appropriate.
      [[nodiscard]] static auto create() -> std::expected<WebClientPtr, ctb::Error>;


      // returns false if webview is already busy loading another page...
      auto requestPage(std::string url, PageLoadedCallback callback) -> bool;

   private:
      std::map<std::string, PageLoadedCallback> m_requests{}; // map request URL to callback for when page-load is completed.
      wxWebView* m_webview{};
      bool m_busy_flag{ false }; // webview has IsBusy(), but it's buggy AF and will always return true after first page is loaded.

      void createWindow();
      void onPageLoaded(wxWebViewEvent& event);
      void onNavigating(wxWebViewEvent& event);
      void onNavigated(wxWebViewEvent& event);
      void onError(wxWebViewEvent& event);
      void onScriptResult(wxWebViewEvent& event);
   };

} // namespace ctb::app