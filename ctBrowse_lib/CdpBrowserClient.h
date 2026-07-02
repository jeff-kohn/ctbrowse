#pragma once

#include "ctb/ctb.h"
#include "utility_win32.h"

#include <atomic>

namespace glz
{
   class websocket_client;
}

namespace ctb
{
   using BrowserClientPtr = std::shared_ptr<glz::websocket_client>;

   enum class CdpStatus
   {
      Stopped = 0,
      Starting,
      Ready,
      Stopping,
   };

   class HeadlessBrowserSession
   {
   public:

      auto healthy() const -> bool;

   private:
      std::string m_session_id{};
      BrowserClientPtr m_browser;
   };




   class HeadlessBrowserManager
   {
   public:
      HeadlessBrowserManager();

      // launches the headless browser, sets up initial session
      void start();

   private:
      //std::atomic<CdpStatus> m_status{ CdpStatus::Stopped };
      BrowserClientPtr m_browser;
   };


}   // namespace ctb
