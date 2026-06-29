#include "../ctBrowse_lib/CdpBrowserClient.h"
#include <catch2/catch_test_macros.hpp>


namespace ctb::tests
{
   TEST_CASE("Launch Headless Browser", "[CdpBrowser]")
   {
      CdpBrowserClient browser{};
      browser.start();
   }

}   // namespace ctb::tests
