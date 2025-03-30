#include "App.h"
#include "tasks.h"

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


   auto makeUpdateCacheTask() -> UpdateCacheTask
   {
      co_return UpdateCacheResult{};
   }


   auto makeHttpGetTask(string_view url, thread_pool& tp, stop_token token) -> HttpRequestTask
   {
      co_await tp.schedule();
      try
      {
         co_return cpr::Get(Url{ url });
      }
      catch (exception& e)
      {
         log::exception(e);
         co_return unexpected{ ResultCode::Error };
      }
   }


}