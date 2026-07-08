#include "../ctBrowse_lib/HeadlessWebClient.h"

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>
#include <print>

namespace ctb::tests
{
   //setupDefaultLogger({ { makeFileSink(log_folder, constants::APP_NAME_SHORT) }, { makeDebuggerSink() } });

   TEST_CASE("Launch Headless Browser", "[CdpBrowser]")
   {
      using std::jthread;
      using namespace asio;
      using namespace webclient;
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

      auto fut = asio::co_spawn(
         *ctx,
         [&browser]() -> asio::awaitable<std::unique_ptr<HeadlessWebClient::TargetSession>>
         {
            auto val = co_await browser.coroCreateSession();
            co_return std::make_unique<HeadlessWebClient::TargetSession>(std::move(val));
         },
         asio::use_future);

      // Blocks this (non-coroutine) thread until the coroutine completes;
      // rethrows any exception the coroutine threw.
      try
      {
         auto maybe_session = fut.get();
         REQUIRE(maybe_session.get() != nullptr);
         auto session = std::move(*maybe_session);
         std::println("Created browser session with id {}", session.sessionId());
      }
      catch(...)
      {
         std::println("coroCreateSession() returned error: {}", packageError().formattedMessage());
      }

      browser.stop();
      while (browser.status() != HeadlessWebClient::Status::Stopped)
      {
         std::this_thread::sleep_for(100ms);
      }

      m_work_guard.reset();
   }


}   // namespace ctb::tests
