/*********************************************************************
 * @file       CredentialWrapper.cpp
 *
 * @brief      Implementation for the class CredentialWrapper
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "cts/CredentialWrapper.h"

#include "Windows.h"
#include "wincred.h"

#include <array>

namespace cts
{

   namespace
   {
      inline CredentialWrapper::ResultCode getCode(DWORD result)
      {
         using enum CredentialWrapper::ResultCode;

         switch (result)
         {
            case NO_ERROR:
               return Success;

            case ERROR_INVALID_FLAGS:
               return ErrorInvalidFlags;

            case ERROR_INVALID_PARAMETER:
               return ErrorInvalidFlags;

            case ERROR_NO_SUCH_LOGON_SESSION:
               return ErrorNoLogonSession;

            case ERROR_CANCELLED:
               return ErrorCanceled;

            default:
               return ErrorUnknown;
         };
      }
   } // namespace


   void CredentialWrapper::swap(CredentialWrapper& other) noexcept
   {
      using std::swap;

      swap(m_target, other.m_target);
      swap(m_message_text, other.m_message_text);
      swap(m_caption_text, other.m_caption_text);
      swap(m_username, other.m_username);
      swap(m_password, other.m_password);
      swap(m_save_checked, other.m_save_checked);
      swap(m_confirmed, other.m_confirmed);
   }


   CredentialWrapper::CredentialWrapper(CredentialWrapper&& other) noexcept
   {
      swap(other);
      other.clear();
   }


   CredentialWrapper& CredentialWrapper::operator=(CredentialWrapper&& other) noexcept
   {
      swap(other);
      other.clear();

      return *this;
   }


   CredentialWrapper::~CredentialWrapper() noexcept
   {
      if (m_save_checked && !m_confirmed)
         // User chose to save but credential was never confirmed, so we don't want to save it.
         confirmCredential(false);

      clear();
   }

   void CredentialWrapper::clear() noexcept
   {
      SecureZeroMemory(m_username.data(), m_username.size());
      SecureZeroMemory(m_password.data(), m_password.size());
      m_confirmed = false;
      m_save_checked = false;
   }


   [[nodiscard]] CredentialWrapper::CredentialResult CredentialWrapper::promptForCredential(unsigned long auth_error)
   {

      m_confirmed = false;
      BOOL save{ allowSave() };
      DWORD retval{ ERROR_SUCCESS };


      auto flags = (allowSave() ? CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_EXPECT_CONFIRMATION
                                : CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST);

      CREDUI_INFOA cui{};
      cui.cbSize = sizeof(CREDUI_INFO);
      cui.hwndParent = NULL;
      cui.pszMessageText = m_message_text.c_str();
      cui.pszCaptionText = m_caption_text.c_str();
      cui.hbmBanner = nullptr;

      retval = CredUIPromptForCredentialsA(&cui, m_target.c_str(), nullptr, auth_error,
                                           m_username.data(), static_cast<ULONG>(m_username.size()),
                                           m_password.data(), static_cast<ULONG>(m_password.size()),
                                           &save, static_cast<DWORD>(flags));
      auto resultCode = getCode(retval);
      m_save_checked = allowSave() && static_cast<bool>(save);

      if (ResultCode::Success == resultCode)
         return Credential{ m_username.data(), m_password.data() };
      else
         return std::unexpected{ resultCode };
   }


   bool CredentialWrapper::confirmCredential(bool valid)
   {
      /// We always set confirmed = true because even if the call fails, calling again in the dtor wouldn't make sense.
      m_confirmed = true;
      return NO_ERROR == CredUIConfirmCredentialsA(m_target.c_str(), valid);
   }


}  // namespace cts
