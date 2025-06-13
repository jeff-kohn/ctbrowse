/*********************************************************************
 * @file       CredentialWrapper.h
 *
 * @brief      Declaration for the class CredentialWrapper
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <expected>
#include <string>
#include <string_view>



namespace ctb
{
   /// @brief Encapsulates a username and password credential
   ///
   /// It's generally best to avoid using password credentials if possible, since it's not possible to use them without exposing
   /// them in memory at least temporarily. But sometimes you have no choice. This class tries to keep password exposure to a
   /// minimum, by only supporting move semantics and zeroing the credential values on destruction (they're also zeroed out in
   /// moved-from objects). string_view's are used to return the values to the caller, which will become invalid on object
   /// destruction (by design). You should keep these objects alive no longer than necessary to minimize exposure of the credentials
   /// in RAM.
   ///
   class CredentialWrapper final
   {
   public:
      /// @brief CredentialWrapper constructor.
      CredentialWrapper(std::string_view cred_name, std::string&& username, std::string&& password, bool save_requested = false);

      /// @brief move constructor
      CredentialWrapper(CredentialWrapper&& other) noexcept;

      /// @brief move-assignment operator
      CredentialWrapper& operator=(CredentialWrapper&& rhs) noexcept;

      /// @brief destructor, zero-overwrites username and password
      ~CredentialWrapper() noexcept;

      /// @brief Checks whether the current credential should be saved.
      /// 
      /// Note that this class doesn't directly support persistence, it's up to the caller
      /// to decide if/how/where to save it, usually after verifying that it's a valid credential.
      /// 
      /// @return true if the credential should be saved, false if not.
      /// 
      auto saveRequested() const -> bool;

      /// @brief Name used to identify this credential when persisting to/from storage.
      /// 
      auto credentialName() const -> const std::string&;

      /// @brief Returns temporary view of credential username
      ///
      /// the returned view is only valid until clear() or this object's destructor is called
      /// 
      auto username() const -> std::string_view;

      /// @brief Returns temporary view of credential password
      ///
      /// the returned view is only valid until clear() or this object's destructor is called
      /// 
      auto password() const -> std::string_view;

      /// @brief Clears the credential values from this object
      ///
      void clear() noexcept;

      /// @brief swap implementation for CredentialWrapper.
      void swap(CredentialWrapper& other) noexcept;

      /// @brief deleted members, this class does not support default construction or copy semantics
      CredentialWrapper() = delete;
      CredentialWrapper(const CredentialWrapper&) = delete;
      CredentialWrapper& operator=(const CredentialWrapper&) = delete;

   private:
      std::string m_cred_name{};
      std::string m_username{};
      std::string m_password{};
      bool        m_cleared{ false };
      bool        m_save_requested{ false };
   };


   /// @brief standalone swap implementation for move semantics 
   inline void swap(CredentialWrapper& left, CredentialWrapper& right)
   {
      left.swap(right);
   }


   /// @brief Result object that can be used to return a CredentialWrapper or an Error from a function
   ///
   using CredentialResult = std::expected<CredentialWrapper, ctb::Error>;




} // namespace ctb
