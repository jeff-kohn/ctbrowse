
#include "ctwin/constants.h"
#include "ctwin/CellarTrackerDownload.h"
#include "ctwin/winapi_util.h"

#include "cpr/cpr.h"
#include "cpr/util.h"

#include <string>
#include <print>


int main()
{
   using namespace std::literals;

   try {
      using namespace ctwin;

      CredentialWrapper cred_wrapper{
         constants::CELLARTRACKER_DOT_COM, true,
         constants::CELLARTRACKER_LOGON_CAPTION,
         constants::CELLARTRACKER_LOGON_TITLE
      };

      // if there's no saved credential, user will be prompted. use a loop since
      // it may take more than one try before user enters the correct credentials
      // (or gives up trying).
      CellarTrackerDownload downloader{};
      CellarTrackerDownload::DownloadResult result{};
      bool prompt_again = true;
      while (prompt_again)
      {
         auto cred = cred_wrapper.promptForCredential();
         if (!cred)
            throw Error{ "Authentication failed." };

         result = downloader.getTableData(cred.value(), CellarTrackerDownload::Table::Notes, CellarTrackerDownload::Format::csv);
         if (!result.has_value() && result.error().error_code == static_cast<int>(HttpStatus::Code::Unauthorized))
            prompt_again = true; // wrong user/pass, try again
         else 
            prompt_again = false;
      }

      if (result.has_value())
      {
         auto& data = result.value();
         util::saveTextToFile(data.data, data.tableName());
      }

      return 0;
   }
    catch (std::exception& ex)
    {
       std::println("\r\nException occurred:{}\r\n", ex.what());
    }
}

