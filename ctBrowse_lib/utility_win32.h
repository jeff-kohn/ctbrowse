#pragma once
#include "ctb/ctb.h"

#include <expected>
#include <memory>

#include <windows.h>

namespace ctb
{
   namespace detail
   {
      struct CloseHandleFunc
      {
         void operator()(HANDLE handle) const noexcept
         {
            if (handle && handle != INVALID_HANDLE_VALUE)
            {
               CloseHandle(handle);
            }
         }
      };

      using UniqueHandlePtr = std::unique_ptr<void, CloseHandleFunc>;

   }   // namespace detail


   struct ProcJobInfo
   {
      using HandlePtr = detail::UniqueHandlePtr;

      HandlePtr process_handle{ INVALID_HANDLE_VALUE };
      DWORD     process_id{ 0 };
   };

   [[nodiscard]] auto createProcessJob(std::string command_line) -> std::expected<ProcJobInfo, ctb::Error>;

}   // namespace ctb
