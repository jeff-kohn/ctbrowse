
#include "ctwin/constants.h"
#include "ctwin/utility.h"

#include "cpr/cpr.h"
#include "cpr/util.h"

#include <string>
#include <print>

int main()
{
   using namespace std::literals;

   try {
      using namespace ctwin;

      auto name = cpr::util::urlEncode("Jeff Kohn");
      auto pwd = cpr::util::urlEncode(getEnvironmentVar(constants::CT_PASSWORD));
      cpr::Url url{ std::format(constants::FMT_HTTP_CT_BASE_URL, name, pwd, "csv", "List") };
      cpr::Header header{ {constants::HTTP_HEADER_XCLIENT, constants::HTTP_HEADER_XCLIENT_VALUE} };

      auto response = cpr::Get(url, header);
      if (cpr::status::is_success(response.status_code))
         saveTextToFile(response.text, fs::path{ "wine_list.csv" }, true);
      else
         return -1;

      return 0;
   }
    catch (std::exception&)
    {
    }
}

