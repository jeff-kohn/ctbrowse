#pragma once

#include "App.h"

#include <ctb/CredentialWrapper.h>
#include <wx/dialog.h>

namespace ctb::app
{
   class CredentialDialog final : public wxDialog
   {
   public:
      CredentialDialog(wxWindow* parent, std::string_view credential_name, std::string_view prompt_msg, bool allow_save);
      CredentialDialog(CredentialDialog&&) = default;
      CredentialDialog& operator=(CredentialDialog&&) = default;
      ~CredentialDialog() override;

      CredentialWrapper getCredential();

      // move only, no default ctor
      CredentialDialog() = delete;
      CredentialDialog(CredentialDialog&) = delete;
      CredentialDialog& operator=(CredentialDialog&) = delete;

   private:
      std::string m_cred_name{};   // only used for getCredential()
      wxString    m_prompt_msg{};
      wxString    m_password_val{};
      wxString    m_username_val{};
      bool        m_allow_save{ true };
      bool        m_save_requested{ false };

      void init();
   };


   // functor that can be used with CredentialManager class for prompting user using CredentialDialog.
   struct CredentialPrompt
   {

   };
}