#include "../ctBrowse_lib/CellarTrackerBrowser.h"
#include "../ctBrowse_lib/senders.h"
#include "../ctBrowse_lib/IoManager.h"

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>
#include <print>

namespace ctb::tests
{
   using namespace ctb::tasks;

   IoManager io_mgr{};

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
                       | let_value(std::bind_front(&CellarTrackerBrowser::downloadLabel, &browser))
                       | then(tasks::decodeResourceContents)
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
