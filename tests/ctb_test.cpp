#include "../ctBrowse_lib/async/IoManager.h"
#include "../ctBrowse_lib/async/senders.h"
#include "../ctBrowse_lib/browser/CellarTrackerBrowser.h"

#include <ctb/model/CtDatasetMgr.h>

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>
#include <print>

namespace ctb::tests
{
   using namespace ctb::senders;

   IoManager io_mgr{};
   //CtDatasetMgr g_dataset_mgr{ DatasetMgrOptions{} };

   TEST_CASE("Launch Headless Browser", "[CdpBrowser]")
   {
      try
      {
         log::setupDefaultLogger({ { log::makeDebuggerSink() }, { log::makeConsoleSink(log::level_enum::debug) } });

         CellarTrackerBrowser browser{ io_mgr.get_context() };

         browser.start();
         while (browser.status() != CellarTrackerBrowser::Status::Ready)
         {
            std::this_thread::sleep_for(100ms);
         }

         int64_t wine_id = 4659587;

         auto pipeline = just(wine_id)
                       | let_value(std::bind_front(&CellarTrackerBrowser::sndDownloadLabel, &browser))
                       | then(senders::decodeResourceContents)
                       | then(
                            [](Buffer buf) -> Buffer
                            {
                               std::println("Received {} bytes for wine label.", buf.size());
                               return buf;
                            });

         auto buffer = stdexec::sync_wait(pipeline);

         browser.stop();
         while (browser.status() != CellarTrackerBrowser::Status::Stopped)
         {
            std::this_thread::sleep_for(100ms);
         }
      }
      catch (...)
      {
         log::exception(packageError());
      }
   }


}   // namespace ctb::tests
