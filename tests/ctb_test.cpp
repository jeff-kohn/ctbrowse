#include "../ctBrowse_lib/HeadlessWebClient.h"

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>


namespace ctb::tests
{
   //setupDefaultLogger({ { makeFileSink(log_folder, constants::APP_NAME_SHORT) }, { makeDebuggerSink() } });

   TEST_CASE("Launch Headless Browser", "[CdpBrowser]")
   {
      using std::jthread;
      using namespace asio;
      using namespace web;
      using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

      auto      ctx = std::make_shared<asio::io_context>(1);
      WorkGuard m_work_guard{ make_work_guard(*ctx) };
      jthread   thread{ [&ctx]
                        {
                         ctx->run();
                      } };

      HeadlessWebClient browser{ ctx };
      browser.start();
      while (browser.status() != HeadlessWebClient::Status::Ready)
      {
         std::this_thread::sleep_for(100ms);
      }

      browser.stop();
      while (browser.status() != HeadlessWebClient::Status::Stopped)
      {
         std::this_thread::sleep_for(100ms);
      }

      m_work_guard.reset();
   }

}   // namespace ctb::tests
