#include "ctb/tasks/tasks.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"


namespace ctb::tasks
{
   using namespace std::literals;
   using std::stop_token;
   using std::string_view;
   using std::vector;

   // NOLINTNEXTLINE(performance-unnecessary-value-param)
   auto runLoadFileTask(fs::path file, stop_token token) noexcept(false) -> FetchFileTask::ReturnType
   {
      // there was originally more to these functions when implementing a coroutine for libcoro, but
      // when that approach was abandoned for std::async() there wasn't much left. I decided to keep the
      // encapsulation because at some point I'll probably want to move to asio/cobalt or P2300/?? or ??/??...
      checkStopToken(token);
      return readBinaryFile(file);
   }

}   // namespace ctb::tasks

