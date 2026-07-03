#include "../ctBrowse_lib/HeadlessWebClient.h"
#include <catch2/catch_test_macros.hpp>
#include <asio/io_context.hpp>

namespace ctb::tests
{
   //setupDefaultLogger({ { makeFileSink(log_folder, constants::APP_NAME_SHORT) }, { makeDebuggerSink() } });

   TEST_CASE("Launch Headless Browser", "[CdpBrowser]")
   {
      web::HeadlessWebClient browser{std::make_shared<asio::io_context>(1)};
      //HeadlessBrowserManager browser{};
      // 
      browser.start();
   }

}   // namespace ctb::tests
