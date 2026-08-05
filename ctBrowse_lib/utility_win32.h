#pragma once
#include "ctb/ctb.h"
#include "ctb/utility.h"
#include "ScopedHandle.h"

#include <expected>
#include <memory>
#include <string_view>

#include <windows.h>

namespace ctb::win32
{
   /// @brief RAII type for HANDLE's that should be freed by calling ::CloseHandle
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

   using UniqueHandlePtr = detail::ScopedHandle<HANDLE, CloseHandleFunc>;


   /// @brief struct containing handles to a process and its owning job object.
   struct ProcessJobHandles
   {
      using HandlePtr = UniqueHandlePtr;

      HandlePtr process_handle{ INVALID_HANDLE_VALUE };
      HandlePtr job_handle{ INVALID_HANDLE_VALUE };
      HandlePtr thread_handle{ INVALID_HANDLE_VALUE };

      auto is_valid() const -> bool
      {
         return process_handle.get()
            and process_handle.get() != INVALID_HANDLE_VALUE
            and job_handle
            and job_handle.get() != INVALID_HANDLE_VALUE
            and thread_handle
            and thread_handle.get() != INVALID_HANDLE_VALUE;
      }
   };

   /// @brief Create a suspended process within a system job
   /// @param command_line
   /// @return
   [[nodiscard]] auto createProcessJob(const fs::path& exe_path, std::string_view args) -> std::expected<ProcessJobHandles, ctb::Error>;

}   // namespace ctb::win32
