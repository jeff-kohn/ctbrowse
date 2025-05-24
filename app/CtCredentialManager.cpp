
#include "CtCredentialManager.h"
#include "dialogs/CredentialDialog.h"

#include <wx/secretstore.h>


namespace ctb::app
{
   namespace
   {
      [[nodiscard]] auto buildCredServiceName(const std::string_view cred_name) -> std::string
      {
         return ctb::format("{}/{}", CtCredentialPersist::CRED_SERVICE_BASE, cred_name);
      }
   }

   [[nodiscard]] auto CtCredentialPersist::credentialExists(std::string_view cred_name) -> bool
   {
      // no query capability in wxSecretStore so we have to try and load it.
      auto result = loadCredential(cred_name);
      return result.has_value();
   }


   [[nodiscard]] auto CtCredentialPersist::loadCredential(std::string_view cred_name) -> CredentialResult
   {
      using std::unexpected;

      auto secret_store = wxSecretStore::GetDefault();
      if (!secret_store.IsOk())
      {
         return unexpected{ Error{ constants::ERORR_STR_NO_SECRET_STORE, Error::Category::NotSupported } };
      }
         
      wxString username{};
      wxSecretValue password{};
      if (secret_store.Load(wxString::FromUTF8(buildCredServiceName(cred_name)), username, password))
      {
         auto pwd_str = password.GetAsString(wxConvUTF8);
         CredentialWrapper cred{ cred_name, username.utf8_string(), pwd_str.utf8_string() };
         wxSecretValue::WipeString(pwd_str);
         return cred;
      }
      return unexpected{ Error{ Error::Category::ArgumentError, constants::FMT_ERORR_NO_CREDENTIAL, cred_name } };
   }


   [[nodiscard]] auto CtCredentialPersist::saveCredential(CredentialWrapper& cred) -> bool
   {
      auto secret_store = wxSecretStore::GetDefault();
      auto username = cred.username();
      auto password = cred.password();

      return secret_store.IsOk() and 
             secret_store.Save(wxString::FromUTF8(buildCredServiceName(cred.credentialName())), 
                               wxString::FromUTF8(username.data(), username.size()),
                               wxSecretValue{ password.size(), password.data() } );
   }


   [[nodiscard]] auto CtCredentialPromptFunc::operator()(const std::string& cred_name, std::string_view prompt_message, bool allow_save) -> CredentialResult
   {
      try 
      {
         CredentialDialog dlg{ wxGetApp().GetMainTopWindow(), cred_name, prompt_message, allow_save };

         if (dlg.ShowModal() == wxID_OK)
         {
            return dlg.getCredential();
         }
         return std::unexpected{ Error{ constants::ERROR_USER_CANCELED, Error::Category::OperationCanceled }};
      }
      catch (...) {
         auto e = packageError();
         log::exception(e);
         return std::unexpected{ e };
      }
   }



} // namespace ctb::app