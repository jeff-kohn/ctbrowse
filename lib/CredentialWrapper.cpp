/*********************************************************************
 * @file       CredentialWrapper.cpp
 *
 * @brief      Implementation for the class CredentialWrapper
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/CredentialWrapper.h"

#if defined(_WIN32)
   #include <Windows.h>
#else
   #include <string.h>
#endif


namespace ctb
{

   CredentialWrapper::CredentialWrapper(std::string_view cred_name, std::string&& username, std::string&& password, bool save_requested) :
      m_cred_name{ cred_name },
      m_username{ std::move(username) },
      m_password{ std::move(password) },
      m_cleared{ false },
      m_save_requested{ save_requested }
   {}


   CredentialWrapper::CredentialWrapper(CredentialWrapper&& other) noexcept
   {
      swap(other);
   }


   CredentialWrapper& CredentialWrapper::operator=(CredentialWrapper&& rhs) noexcept
   {
      swap(rhs);
      rhs.clear();
      return *this;
   }


   CredentialWrapper::~CredentialWrapper() noexcept
   {
      if (!m_cleared) clear();
   }


   auto CredentialWrapper::saveRequested() const -> bool
   {
      return m_save_requested;
   }


   /// @brief Name used to identify this credential when persisting to/from storage.
   /// 
   auto CredentialWrapper::credentialName() const -> const std::string&
   {
      return m_cred_name;
   }


   /// @brief Returns temporary view of credential username
   ///
   /// the returned view is only valid until clear() or this object's destructor is called
   /// 
   auto CredentialWrapper::username() const -> std::string_view
   {
      return m_username;
   }


   /// @brief Returns temporary view of credential password
   ///
   /// the returned view is only valid until clear() or this object's destructor is called
   /// 
   auto CredentialWrapper::password() const -> std::string_view
   {
      return m_password;
   }


   /// @brief Clears the credential values from this object
   ///
   void CredentialWrapper::clear() noexcept
   {
#if defined(_WIN32)
      SecureZeroMemory(m_username.data(), m_username.size());
      SecureZeroMemory(m_password.data(), m_password.size());
#else
      memset_explicit(m_username.data(), m_username.size());
      memset_explicit(m_password.data(), m_password.size());
#endif
      m_cleared = true;
   }


   void CredentialWrapper::swap(CredentialWrapper& other) noexcept
   {
      using std::swap;

      swap(m_cred_name, other.m_cred_name);
      swap(m_username, other.m_username);
      swap(m_password, other.m_password);
      swap(m_cleared, other.m_cleared);
      swap(m_save_requested, other.m_save_requested);
   }



}  // namespace ctb
