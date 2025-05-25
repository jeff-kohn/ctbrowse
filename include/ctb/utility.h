/*******************************************************************
 * @file utility.h
 *
 * @brief Header file for some helper templates/functions
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <filesystem>


namespace ctb
{
   namespace fs = std::filesystem;


   /// @brief read a binary file (up to max_size bytes in size) into a char buffer
   /// 
   /// @throws ctb::Error, possibly other std::exception-derived if file can't be read or is larger than max_size
   /// 
   auto readBinaryFile(const fs::path& file_path, uint32_t max_size = constants::ONE_MB) noexcept(false) -> Buffer;


   /// @brief Save binary data to a file.
   /// @throws ctb::Error, possibly other std::exception-derived
   /// 
   auto saveBinaryFile(const fs::path& file_path, BufferSpan buf, bool overwrite = false) noexcept(false) -> void;


   /// @brief just dump some text to a file (no encoding or formatting applied).
   ///
   /// if the file_path has a parent directory and it doesn't exist, an attempt will be made to 
   /// create it. 
   /// 
   /// @throws ctb::Error, possibly other std::exception-derived if file can't be written or already exists
   ///         and overwrite = false;
   /// 
   auto saveTextToFile(fs::path file_path, std::string_view text, bool overwrite = false) noexcept(false) -> void;


   /// @brief Get a view/substring of just the filename from a string containing a path
   /// 
   /// The returned string_view is only valid for the lifetime of fq_path. This 
   /// function does not modify fq_path; it only takes a non-const ref to prevent 
   /// passing an rvalue (since that would be unsafe)
   ///
   auto viewFilename(std::string& fq_path) noexcept -> std::string_view;


   /// @brief Currently-supported code pages.
   constexpr unsigned int CP_ISO_LATIN_1  = 28591;
   constexpr unsigned int CP_WINDOWS_1252 = 1252;

   /// @brief convert text to UTF8 from other narrow/multi-byte encoding.
   ///
   /// see https://learn.microsoft.com/en-us/windows/win32/Intl/code-page-identifiers
   /// for a list of code page id's.
   /// 
   /// @return the converting string if successful, std::nullopt if not.
   /// 
   [[nodiscard]] auto toUTF8(const std::string& text, unsigned int from_code_page = CP_WINDOWS_1252) -> MaybeString;


   /// @brief convert text to UTF8 from other narrow/multi-byte encoding.
   ///
   /// see https://learn.microsoft.com/en-us/windows/win32/Intl/code-page-identifiers
   /// for a list of code page id's.
   /// 
   /// @return the converting string if successful, std::nullopt if not.
   /// 
   [[nodiscard]] auto fromUTF8(const std::string& utf8_text, unsigned int to_code_page = CP_WINDOWS_1252) -> MaybeString;


   /// @brief  Expand environment variables in place
   /// 
   /// In the case when the passed string does not contain any environment vars, this function
   /// returns without any allocation or copying, which gives it a slight performance edge over
   /// expandEnvironmentVars() if you're working with strings that may or may not contain any vars.
   /// 
   /// @return true if successful, false if unsuccessful in which case the parameter 'text' will be unmodified
   /// 
   auto tryExpandEnvironmentVars(std::string& text) -> bool;


   /// @brief Expand environment variables and return result
   ///
   /// while this overload is convenient, it has the overhead of an unnecessary copy
   /// when the passed string has no vars to expand.
   template<StringOrStringViewType Str>
   inline auto expandEnvironmentVars(Str&& text) -> std::string
   {
      std::string result{std::forward<Str>(text)};
      tryExpandEnvironmentVars(result);
      return result;
   }

} // namespace ctb