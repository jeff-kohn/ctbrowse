#include "CdpBrowserClient.h"


namespace ctb
{
   constexpr auto EDGE_PATH = R"(C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe)";
   constexpr auto FMT_EDGE_ARGS = " --remote-debugging -port=9222 --disable-gpu --user-data-dir=\"{}\"";


   void CdpBrowserClient::start()
   {
      m_status.store(CdpStatus::Starting);

      // launch the browser process.
      auto proc_result = createProcess(format("{} {}", EDGE_PATH, EDGE_ARGS));
      if (!proc_result) throw proc_result.error();

      m_edge_handle.swap(proc_result->process_handle);
   }
}
