/*********************************************************************
* @file       CredentialManager.h
*
* @brief      Declaration for the class CredentialManager
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/CredentialWrapper.h"

#include <string>
#include <string_view>

namespace ctb
{

   /// @brief Concept for a policy class that can persist a CredentialWrapper. It should support the following
   ///        functions:
   ///   
   ///  auto credentialExists(std::string_view cred_name) -> bool
   ///  auto loadCredential(std::string_view cred_name) -> CredentialResult
   ///  auto saveCredential(CredentialWrapper& cred) -> bool
   /// 
   template <typename T>
   concept CredentialPersistPolicy = requires (T t, std::string_view sv, bool b, CredentialResult result)
   {
      t = T{};
      b = t.credentialExists(sv);
      result = t.loadCredential(sv);
      b = t.saveCredential(result.value());
   };


   /// @brief Concept for a callable that can interactively prompt a user to get a CredentialWrapper. It should 
   ///        be compatible with:
   /// 
   ///        std::function<CredentialResult(std::string_view cred_name, bool allow_save)>
   ///   
   template <typename T>
   concept CredentialPromptFunc = requires (T t, std::string_view s, CredentialResult cred)
   {
      t = T{};
      cred = t(s, s, false);
   };


   /// @brief This CredentialPersistPolicy policy doesn't actually persist anything and always returns false/error
   ///
   struct CredPersistDisabled
   {
      auto credentialExists(std::string_view cred_name) -> bool
      {
         return false;
      }

      auto loadCredential(std::string_view& cred_name) -> CredentialResult
      {
         return std::unexpected{ Error{ "Credential not loaded because persistence is disabled", Error::Category::NotSupported }};
      }

      auto saveCredential(CredentialWrapper& cred) -> bool
      {
         return false;
      }
   };


   /// @brief CredentialManager class
   /// 
   template<CredentialPromptFunc CredPromptT, CredentialPersistPolicy CredPersistT = CredPersistDisabled>
   class CredentialManager
   {
   public:
      using CredPersistType = CredPersistT;
      using CredPromptType  = CredPromptT;

      static inline constexpr int MAX_USERNAME_LENGTH = 513; /// max username length not including null terminator
      static inline constexpr int MAX_PASSWORD_LENGTH = 256; /// max password length not including null terminator


      CredentialManager(CredPromptType prompt) : m_prompt(std::move(prompt))
      {}
         
      CredentialManager() = default;
      ~CredentialManager() noexcept = default;


      /// @brief Checks whether a credential with the specified name is available be loaded
      /// @return true if the credential can be loaded, false if not
      /// 
      auto credentialExists(std::string_view cred_name) -> bool
      {
         return m_persist.credentialExists(cred_name);
      }

      /// @brief Load the requested credential from persistent storage if available, optionally prompting user if not found.
      /// 
      /// @param cred_name - the name of the credential to load
      /// @param prompt_message - true if we should interactively prompt user if credential couldn't be loaded.
      /// @return The requested credential if successful, a ctb::Error if unsuccessful.
      /// 
      auto loadCredential(std::string_view cred_name, std::string_view prompt_msg, bool allow_save = true) -> CredentialResult
      {
         return loadCredential(cred_name)
                .or_else([&, this](Error) -> CredentialResult { return promptCredential(cred_name, prompt_msg, allow_save); });
      }

      /// @brief Load the requested credential from persistent storage if available, optionally prompting user if not found.
      /// 
      /// @param cred_name - the name of the credential to load
      /// @param prompt_message - true if we should interactively prompt user if credential couldn't be loaded.
      /// @return The requested credential if successful, a ctb::Error if unsuccessful.
      /// 
      auto loadCredential(std::string_view cred_name) -> CredentialResult
      {
         return m_persist.loadCredential(cred_name);
      }
            
      /// @brief Interactively prompt for a credential
      /// @param cred_name - the name of the credential to load
      /// @param prompt_message - message to display to the user when prompting
      /// @param allow_save - if true, user will be given the option to request that the credential be saved.
      /// @return - the requested credential if successful, a ctb::Error if unsuccessful.
      /// 
      auto promptCredential(std::string_view cred_name, std::string_view prompt_message, bool allow_save) -> CredentialResult
      {
         return m_prompt(cred_name, prompt_message, allow_save);
      }

      /// @brief - Save the credential to persistent storage (if supported)
      /// 
      /// If cred.saveRequested() == true, the credential will be saved to persistent storage if this
      /// CredentialManager supports it. If cred.saveRequested() == false, this call will be a no-op; so
      /// caller doesn't need to explicitly check and can always just call saveCredential()
      /// 
      /// @return - true if successful, false if unsuccessful
      /// 
      auto saveCredential(CredentialWrapper& cred) -> bool
      {
         if (cred.saveRequested())
         {
            return m_persist.saveCredential(cred);
         }
         return false;
      }

      /// @brief Delete a credential from persistent storage
      /// @param cred_name 
      /// @return 
      auto deleteCredential(const std::string_view cred_name) -> int
      {
         if (credentialExists(cred_name))
         {
            return m_persist.deleteCredential(cred_name);
         }
         return false;
      }

   private:
      CredPersistType m_persist{};
      CredPromptType  m_prompt{};
   };


} // namespace ctb
