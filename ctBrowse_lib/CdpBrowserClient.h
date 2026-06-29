#pragma once

#include "ctb/ctb.h"
#include "utility_win32.h"

#include <atomic>
#include <glaze/net/websocket_client.hpp>

namespace ctb
{
   enum class CdpStatus
   {
      Stopped = 0,
      Starting,
      Ready,
      Stopping,
   };

   class CdpBrowserClient
   {
   public:
      // launches the headless browser, sets up initial sesssion
      void start();

   private:
      std::atomic<CdpStatus>         m_status{ CdpStatus::Stopped };
      glz::websocket_client          m_browser;
      CreateProcessResult::HandlePtr m_edge_handle{};
   };


}   // namespace ctb
