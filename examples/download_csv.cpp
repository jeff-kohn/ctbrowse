/*********************************************************************
 * @file       download_csv.h
 *
 * @brief      example showing how to use ctBrowse_lib to download raw
 *             table data from CellarTracker.com
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/constants.h"
#include "ctb/table_download.h"
#include "ctb/utility.h"
#include "external/HttpStatusCodes.h"
#include "ctb/CredentialManager.h"
#include <cpr/cpr.h>
#include <cpr/util.h>

#include <string>
#include <print>


int main()
{
   using namespace std::literals;

   try {
      using namespace ctb;

      CredentialWrapper cred{ constants::CELLARTRACKER_DOT_COM, "Jeff Kohn", "lkj243df" };

      auto result = downloadRawTableData(cred, TableId::List, DataFormatId::csv);
      if (!result)
         throw result.error();
      
      saveTextToFile(result->data, result->tableName());

      //// if there's no saved credential, user will be prompted. use a loop since
      //// it may take more than one try before user enters the correct credentials
      //// (or gives up trying).
      //DownloadResult result{};
      //bool prompt_again = true;
      //while (prompt_again)
      //{
      //   auto cred = cred_wrapper.promptForCredential();
      //   if (!cred)
      //      throw Error{ "Authentication failed." };

      //   result = downloadRawTableData(cred.value(), TableId::List, DataFormatId::csv);
      //   if (!result.has_value() && result.error().error_code == static_cast<int>(HttpStatus::Code::Unauthorized))
      //      prompt_again = true; // wrong user/pass, try again
      //   else 
      //      prompt_again = false;
      //}

      //if (result.has_value())
      //{
      //   auto& data = result.value();
      //   saveTextToFile(data.data, data.tableName());
      //}

      return 0;
   }
    catch (std::exception& ex)
    {
       std::println("\r\nException occurred:{}\r\n", ex.what());
    }
}

