/*********************************************************************
 * @file       CredentialManager.cpp
 *
 * @brief      Implementation for the class CredentialWrapper
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/CredentialManager.h"

#include "Windows.h"
#include "wincred.h"

#include <array>

namespace ctb
{

   auto CredentialPromptFuncWinApi::operator()(const std::string& cred_name, std::string_view prompt_message, bool /*allow_save*/) -> CredentialResult
   {

#if !defined(_WIN32_WINNT)
      assert(false and "Not yet implemented for other platforms.");
      return {};
#endif
      try {
         BOOL  save{ FALSE }; // ignore allow_save fn param, winapi won't save anyway without reg hack
         DWORD auth_error_code{ 0 };
         DWORD flags{ CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST };

         CREDUI_INFOA cui{
            .cbSize = sizeof(CREDUI_INFO),
            .hwndParent = nullptr,
            .pszMessageText = "Enter a credential for CellarTracker.com",
            .pszCaptionText = "Enter Credential",
            .hbmBanner = nullptr
         };

         using cred_buf = std::array<char, std::max(CREDUI_MAX_USERNAME_LENGTH + 1, CREDUI_MAX_PASSWORD_LENGTH + 1)>;
         cred_buf username{ {'\0'} };
         cred_buf password{ {'\0'} };
         auto result = CredUIPromptForCredentialsA(&cui, cred_name.c_str(), nullptr, auth_error_code,
                                                   username.data(), static_cast<ULONG>(username.size()),
                                                   password.data(), static_cast<ULONG>(password.size()),
                                                   &save, flags);

         if (NO_ERROR == result)
            return CredentialWrapper{ cred_name, username.data(), password.data() };
         else
            throw ctb::Error{ static_cast<int64_t>(result), "Login Failed." };
      }
      catch (...) {
         auto e = packageError();
         log::exception(e);
         return std::unexpected{ e };
      }
   }


}  // namespace ctb





