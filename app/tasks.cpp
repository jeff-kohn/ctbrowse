#include "App.h"
#include "tasks.h"

#include <ctb/utility_http.h>
#include <cpr/cpr.h>


namespace ctb::tasks
{
   using coro::sync_wait;
   using coro::thread_pool;
   using cpr::Url;
   using std::exception;
   using std::expected;
   using std::stop_token;
   using std::string_view;
   using std::vector;
   using std::unexpected;


   auto makeFileLoadTask(fs::path file, thread_pool& tp, stop_token token) -> FetchImageTask
   {
      co_await tp.schedule();
      try
      {
         if (token.stop_requested()) co_return std::unexpected{ ResultCode::Aborted };

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


   auto makeUpdateCacheTask(thread_pool& tp, stop_token token) -> UpdateCacheTask
   {
      //co_await tp.schedule();
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


   auto makeHttpGetTask(string_view url, thread_pool& tp, stop_token token) -> HttpRequestTask
   {
      co_await tp.schedule();
      try
      {
         if (token.stop_requested()) co_return std::unexpected{ ResultCode::Aborted };

         auto response = cpr::Get(Url{ url }, getDefaultHeaders() );
         auto result = validateResponse(response);
         if (!result)
            throw result.error();

         co_return response;
      }
      catch (exception& e)
      {
         log::exception(e);
         co_return unexpected{ ResultCode::Error };
      }
   }


}