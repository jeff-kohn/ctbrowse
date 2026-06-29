#include "utility_win32.h"
#include "ctb/utility.h"

#include <memory>
#include <string>
#include <windows.h>


namespace ctb
{

   ctb::Error getLastError()
   {}

   auto createProcess(std::string command_line) -> std::expected<CreateProcessResult, ctb::Error>
   {
      PROCESS_INFORMATION pi{};
      STARTUPINFO         si{};
      si.cb = sizeof(si);

      ProcJobInfo info{};
      auto job_handle = CreateJobObject(nullptr, nullptr);
      if (!job_handle)
      {
         return std::unexpected{
            Error{ GetLastError(), Error::Category::GenericError, "CreateProcess Failed" }
         };
      }

      bool success = CreateProcess(nullptr,               // Application Name (null means parsed from command_line)
                                    command_line.data(),   // Mutable command line buffer
                                    nullptr,               // Process security attributes
                                    nullptr,               // Thread security attributes
                                    FALSE,                 // Inherit handles
                                    0,                     // Creation flags
                                    nullptr,               // Environment block
                                    nullptr,               // Current directory
                                    &si,                   // Startup info
                                    &pi                    // Receives process information
      );

      if (success) return CreateProcessResult{ CreateProcessResult::HandlePtr{ pi.hProcess }, pi.dwProcessId };

      return std::unexpected{ Error{ GetLastError(), Error::Category::GenericError, "CreateProcess Failed" }};
   }


}   // namespace ctb
