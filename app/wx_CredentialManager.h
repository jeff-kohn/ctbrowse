#pragma once

#include "App.h"

#include <ctb/CredentialManager.h>

namespace ctb::app
{
   /// @brief wxWidgets-derived implementation of ctb::CredentialPersistPolicy
   ///
   struct wx_CredentialPersist
   {
      static inline constexpr const char* CRED_SERVICE_BASE = constants::APP_NAME_SHORT;

      auto credentialExists(std::string_view cred_name) -> bool;
      
      [[nodiscard]] auto loadCredential(std::string_view cred_name) -> CredentialResult;
      [[nodiscard]] auto saveCredential(CredentialWrapper& cred) -> bool;
   };

   /// @brief wxWidgets-derived implementation of ctb::CredentialPromptFunc
   ///
   struct wx_CredentialPromptFunc
   {
      [[nodiscard]] auto operator()(std::string_view cred_name, std::string_view prompt_message, bool allow_save) -> CredentialResult;
   };


   /// @brief wxWidgets-derived instantiation of CredentialManager that provides secure secret
   ///        storage and interactive logon prompting.
   ///
   using wx_CredentialManager =  CredentialManager<wx_CredentialPromptFunc, wx_CredentialPersist>;
}