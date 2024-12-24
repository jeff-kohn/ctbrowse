
#include "ctwin/constants.h"
#include "ctwin/environment.h"
//#include "cpr/cpr.h"

int main()
{
   using namespace std::literals;

   try {
      using namespace ctwin;

      auto name = "Jeff Kohn"sv;
      auto pwd = getEnvironmentVariable(constants::CT_PASSWORD);
      //cpr::Header header{
      //   { constants::REST_HEADER_XCLIENT, constants::REST_HEADER_XCLIENT_VALUE }
      //};

      return 0;
   }
    catch (std::exception&)
    {
    }
}

