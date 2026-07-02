#include "utility_win32.h"
#include "auto_handle.h"
#include "ctb/utility.h"

#include <fmt/std.h>
#include <string>

#include <jobapi2.h>
#include <windows.h>

namespace ctb::win32
{

   struct LocalFreeStrFunc
   {
      void operator()(LPSTR str_buf) const noexcept
      {
         if (str_buf)
         {
            LocalFree(str_buf);
         }
      }
   };

   using LocalFreeStr = detail::auto_handle<LPSTR, LocalFreeStrFunc>;


   ctb::Error getLastError()
   {
      auto  error  = ::GetLastError();
      LPSTR buffer = nullptr;
      if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         nullptr,
                         error,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         reinterpret_cast<LPSTR>(&buffer),
                         0,
                         nullptr))
      {
         LocalFreeStr     str_buffer(buffer);
         std::string_view err_msg{ buffer };
         while (!err_msg.empty() && (err_msg.back() == '\n' || err_msg.back() == '\r'))
         {
            err_msg.remove_suffix(1);
         }
         return ctb::Error{ error, Error::Category::GeneralError, "{}", std::string{ err_msg } };
      }
      else
      {
         return ctb::Error{ error, Error::Category::GeneralError, "System error {} occurred.", error };
      }
   }


   [[nodiscard]] auto createProcessJob(const fs::path& exe_path, std::string_view args) -> std::expected<ProcessJobHandles, ctb::Error>
   {
      using HandlePtr = ProcessJobHandles::HandlePtr;

      // first create a job object and configure it to terminate its processes when the handle is close
      HandlePtr job_handle{ ::CreateJobObject(nullptr, nullptr) };
      if (!job_handle)
      {
         return std::unexpected{ getLastError() };
      }

      JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobLimits{};
      jobLimits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
      if (!::SetInformationJobObject(job_handle.get(), JobObjectExtendedLimitInformation, &jobLimits, sizeof(jobLimits)))
      {
         return std::unexpected{ getLastError() };
      }

      // now create the suspended process
      PROCESS_INFORMATION pi{};
      STARTUPINFO         si{};
      si.cb = sizeof(si);
      std::string cmd_line = ctb::format("\"{}\" {}", exe_path, args);
      if (!CreateProcess(nullptr,                            // Application Name (null means parsed from command_line)
                         cmd_line.data(),                    // Mutable command line buffer
                         nullptr,                            // Process security attributes
                         nullptr,                            // Thread security attributes
                         FALSE,                              // Inherit handles
                         CREATE_SUSPENDED,                   // Creation flags
                         nullptr,                            // Environment block
                         nullptr,                            // Current directory
                         &si,                                // Startup info
                         &pi                                 // Receives process information
                         ))
      {
         return std::unexpected{ getLastError() };
      }

      // Create the return value, then add the process to the job and resume its thread before returning.
      ProcessJobHandles proc_info{ .process_handle = HandlePtr{ pi.hProcess },
                                   .job_handle     = std::move(job_handle),
                                   .thread_handle  = HandlePtr{ pi.hThread } };

      if (!AssignProcessToJobObject(proc_info.job_handle.get(), proc_info.process_handle.get()))
      {
         auto err = getLastError();
         TerminateProcess(proc_info.process_handle.get(), EXIT_FAILURE);
         return std::unexpected{ std::move(err) };
      }
      ::ResumeThread(proc_info.thread_handle.get());

      return proc_info;
   };
}   // namespace ctb::win32


