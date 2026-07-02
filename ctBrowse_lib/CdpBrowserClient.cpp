#include "CdpBrowserClient.h"
#include "ctb/utility.h"
#include "utility_win32.h"

#include <glaze/net/websocket_client.hpp>

#include <string_view>


namespace ctb
{
   constexpr auto EDGE_PATH = R"(C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe)"sv;
   constexpr auto EDGE_ARGS = "--headless --no-first-run --no-default-browser-check --disable-sync "
                              "--remote-debugging-port=9222 --disable-gpu "
                              "--user-data-dir=\"%LOCALAPPDATA%\\ctBrowse for Windows\\EBWebView\""sv;

   HeadlessBrowserManager::HeadlessBrowserManager() : m_browser{ std::make_shared<glz::websocket_client>() }
   {}

   void HeadlessBrowserManager::start()
   {
      //m_status.store(CdpStatus::Starting);

      auto proc_handles = win32::createProcessJob(EDGE_PATH, expandEnvironmentVars(EDGE_ARGS));
   }
}   // namespace ctb
