#include <catch2/catch_test_macros.hpp>
#include <ctb/download.h>

#include <chrono>
#include <print>


namespace ctb::tests
{

   using namespace std::literals;


   TEST_CASE("download_table", "[rest_helpers]")
   {
      CredentialWrapper cred{ "test", "", "" };

      //downloadTableAsync(
      //   cred,
      //   [](DownloadResult result)
      //   {
      //      if (result)
      //      {
      //         std::println("Got table {}, size {} bytes.", result->tableName(), result->data.size());
      //      }
      //      else
      //      {
      //         std::println("Got an error! {}", result.error().formattedMessage());
      //      }
      //   },
      //   TableId::Pending);

      std::this_thread::sleep_for(1s);
   }

}   // namespace ctb::tests
