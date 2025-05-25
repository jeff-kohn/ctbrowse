/*********************************************************************
 * @file       download_csv.h
 *
 * @brief      example showing how to use ctBrowse_lib to download raw
 *             table data from CellarTracker.com
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include <ctb/CredentialManager.h>
#include <ctb/table_download.h>
#include <ctb/utility.h>
#include <external/HttpStatusCodes.h>

#include <string>
#include <print>


int main()
{
   using namespace std::literals;

   try {
      using namespace ctb;

      CredentialManager<CredentialPromptFuncWinApi> cred_mgr{};

      DownloadResult result{};
      bool prompt_again = true;
      while (prompt_again)
      {
         auto cred = cred_mgr.promptCredential("CellarTracker.com", "Enter CellarTracker Credentials:", false);
         if (!cred)
            return 0;

         result = downloadRawTableData(cred.value(), TableId::List, DataFormatId::csv, nullptr, false);
         prompt_again = !result and result.error().error_code == static_cast<int>(HttpStatus::Code::Unauthorized);
      }

      if (!result.has_value())
         throw result.error();
      
      // testing UTF-8/Win-1252 round-trip conversion
      auto& table = result.value();
      saveTextToFile("win-1265.txt", table.data, true);
      auto utf_data = toUTF8(table.data, CP_WINDOWS_1252).value_or("conversion failed");
      saveTextToFile("utf-8.txt", utf_data, true);
      auto round_trip_data = fromUTF8(utf_data, CP_WINDOWS_1252).value_or("conversion failed");
      saveTextToFile("round-trip.txt", round_trip_data, true);

      if (round_trip_data == table.data)
         std::println("Well that was unexpected, got a match!");
      else
         std::println("told you!");

      return 0;
   }
   catch (std::exception& ex)
   {
      std::println("\r\nException occurred:{}\r\n", ex.what());
   }
}

