#include "App.h"
#include "tasks.h"

namespace ctb::tasks
{
   using coro::sync_wait;
   using coro::thread_pool;
   using std::exception;
   using std::expected;
   using std::stop_token;
   using std::vector;
   using std::unexpected;


   auto makeFileLoadTask(thread_pool& tp, stop_token token, fs::path file) -> FetchImageTask
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
      return UpdateCacheTask{};
   }


   auto makeHttpRequestTask() -> HttpRequestTask
   {
      return HttpRequestTask{};
   }


}