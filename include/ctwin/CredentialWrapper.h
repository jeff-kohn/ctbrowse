/*********************************************************************
 * @file       CredentialWrapper.h
 *
 * @brief      Declaration for the class CredentialWrapper
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include <array>
#include <expected>
#include <string>
#include <string_view>


namespace ctwin
{
   /// @brief class to encapsulate using the WinAPI to prompt for (and optionally save) generic username/password credentials.
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
      static inline constexpr int MAX_USERNAME_LENGTH = 513; /// max username length not including null terminator
      static inline constexpr int MAX_PASSWORD_LENGTH = 256; /// make password length not including null terminator


      /// @brief struct containing the username and password result from a call to PromptForCredential()
      ///
      /// any usage of a Credential object after the CredentialWrapper that it refers to is destroying results
      /// in undefined behavior since this object doe not have its own copy of the username nad password.
      struct Credential
      {
         std::string_view username;
         std::string_view password;
      };


      /// @brief  Enumeration for possible return codes from PromptForCredential()
      enum class ResultCode
      {
         Success,                /// Success
         ErrorCanceled,          /// User canceled the dialog 
         ErrorInvalidFlags,      /// One or more invalid flags supplied
         ErrorInvalidParameter,  /// One or more invalid parameter values were supplied
         ErrorNoLogonSession,    /// The credential manager cannot be used; try again with allow_save = false
         ErrorNotFound,          ///
         ErrorUnknown            /// ??? shoudl never occur
      };


      /// @brief type returned from PromptForCredential
      using CredentialResult = std::expected<Credential, ResultCode>;


      /// @brief              construct a CredentialWrapper for the specified target/identifier
      /// @param target       the name that will be used to save the credential if the user chooses to save it
      /// @param allow_save   if true, user will have option to save credential, and if a matching credential has
      ///                     previously been saved it will be used without prompting the user.
      ///
      /// if allow_save is true and the target credential has been previously saved, the saved credential will
      /// be used without prompting the user
      /// 
      CredentialWrapper(std::string target, bool allow_save) :
         m_target(std::move(target)),
         m_allow_save(allow_save)
      {}


      /// @brief              construct a CredentialWrapper by specifying target, message, and caption
      /// @param target       the name that will be used to save the credential if the user chooses to save it
      /// @param allow_save   if true, user will have option to save credential, and if a matching credential has
      ///                     previously been saved it will be used without prompting the user.
      /// @param message_text The message that will be displayed in the prompt dialog (if displayed)
      /// @param caption_text The caption that will be displayed in the title bar of the prompt dialog.
      ///
      /// if allow_save is true and the target credential has been previously saved, the saved credential will
      /// be used without prompting the user
      /// 
      CredentialWrapper(std::string target, bool allow_save, std::string message_text, std::string caption_text) :
         m_target(std::move(target)),
         m_message_text(std::move(message_text)),
         m_caption_text(std::move(caption_text)),
         m_allow_save(allow_save)
      {}


      /// @brief move constructor
      CredentialWrapper(CredentialWrapper&&) noexcept;


      /// @brief move-assignment operator
      CredentialWrapper& operator=(CredentialWrapper&&) noexcept;


      /// @brief destructor, zero-overwrites username and password
      ~CredentialWrapper() noexcept;


      /// @brief indicates whether this CredentialWrapper allows saving successfully validated credentials.
      bool allowSave() const noexcept
      {
         return m_allow_save;
      }


      /// @brief string containing a short message to display in the dialog box (if displayed)
      const std::string& messageText() const { return m_message_text; }
      CredentialWrapper& setMessageText(std::string_view text)
      {
         m_message_text = text;
         return *this;
      }


      /// @brief string containing the title for the dialog box (if displayed)
      std::string_view captionText() const { return m_caption_text; }
      CredentialWrapper& setCaptionText(std::string_view text)
      {
         m_caption_text = text;
         return *this;
      }


      /// @brief zero-overwrites the username/password stored in this object.
      ///
      /// any previously-returned Credential objects will be promptForCredential will be impacted by calling this,
      /// since they contain string_views pointing to the same buffer we're zero-overwriting.
      void clear() noexcept;


      /// @brief                 prompt user for a credential.
      /// @param auth_error_code should be zero for 1st prompt, can be a standard error code for subsequent prompts
      /// @return                the requested credential is successful, and error code if not.
      ///
      /// the auth_error_code can be set to a standard error code such as ERROR_LOGON_FAILURE if you are re-prompting
      /// because the initially-entered credentials did not work, for instance.
      /// 
      [[nodiscard]] CredentialResult promptForCredential(unsigned long auth_error_code = 0);
      

      /// @brief indicate whether the credential returned by PromptForCredential() was valid or not.
      ///
      /// If PromptForCredential() was called with allow_save == true, you must call this method after using/testing the credential
      /// to indicate whether the Credential was valid and should be saved (true) or whether it was invalid and should be discarded.
      /// If not called explicitly, ConfirmCredential(false) will be called by the destructor since it is required by Windows to
      /// ensure resources are properly cleaned up.
      ///                 
      bool confirmCredential(bool valid);


      /// @brief deleted members, this class does not support default construction or copy semantics
      CredentialWrapper() = delete;
      CredentialWrapper(const CredentialWrapper&) = delete;
      CredentialWrapper& operator=(const CredentialWrapper&) = delete;

      /// @brief swap implementation for CredentialWrapper.
      void swap(CredentialWrapper& other) noexcept;

   private:
      std::string m_target{};
      std::string m_message_text{};
      std::string m_caption_text{};
      std::array<char, MAX_USERNAME_LENGTH + 1> m_username{};
      std::array<char, MAX_PASSWORD_LENGTH + 1> m_password{};

      bool m_allow_save{ false }; 
      bool m_save_checked{ false };        /// true if user checked the "save" box, false otherwise
      bool m_confirmed{ false };           /// will be true if ConfirmCredential() has been called, regardless of whether true or false was passed.
   };


   /// @brief standalone swap implementation for move semantics 
   inline void swap(CredentialWrapper& left, CredentialWrapper& right)
   {
      left.swap(right);
   }


} // namespace ctwin
