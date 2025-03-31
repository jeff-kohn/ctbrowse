#include "App.h"
#include "tasks.h"

#include <ctb/utility_http.h>
#include <cpr/cpr.h>


namespace ctb::tasks
{
   using cpr::Url;
   using std::exception;
   using std::expected;
   using std::stop_token;
   using std::string_view;
   using std::vector;
   using std::unexpected;


   auto makeLoadFileTask(fs::path file, stop_token token) -> FetchImageTask
   {
      try
      {
         checkStopToken(token);

         vector<char> buf{};
         readBinaryFile(file, buf);
         co_return buf;
      }
      catch (exception& e)
      {
         log::exception(e);
         co_return unexpected{ ResultCode::Error };
      }
   }


   auto makeUpdateCacheTask(stop_token token) -> UpdateCacheTask
   {
      //try
      //{
      //   //if (token.stop_requested()) co_return unexpected{ ResultCode::Aborted };

      //   co_return UpdateCacheResult{};
      //}
      //catch (exception& e)
      //{
      //   log::exception(e);
      //   co_return unexpected{ ResultCode::Error };
      //}
      co_return {};
   }




   auto makeHttpGetTask(string_view url, stop_token token) -> HttpRequestTask
   {
      try
      {
         checkStopToken(token);

         auto response = cpr::Get(Url{ url }, getDefaultHeaders() );
         auto result = validateResponse(response);
         if (!result)
            co_return unexpected{ Error(result.error()) };

         co_return response;
      }
      catch (exception& e)
      {
         log::exception(e);
         co_return unexpected{ Error{ e.what() } };
      }
   }


}