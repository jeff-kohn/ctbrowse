/*******************************************************************
 * @file utility.h
 *
 * @brief 2 file for some helper templates/functions
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <boost/algorithm/string/predicate.hpp>
#include <filesystem>
#include <optional>

namespace ctb
{

   // can be used with monadic fns
   inline auto to_unsigned(int32_t val) -> uint32_t
   {
      return static_cast<uint32_t>(val);
   }

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
   auto saveTextToFile(const fs::path& file_path, std::string_view text, bool overwrite = false) noexcept(false) -> void;


   /// @brief Get a view/substring of just the filename from a string containing a path
   ///
   /// The returned string_view is only valid for the lifetime of fq_path. This
   /// function does not modify fq_path; it only takes a non-const ref to prevent
   /// passing an rvalue (since that would be unsafe)
   ///
   auto viewFilename(std::string& fq_path) noexcept -> std::string_view;


   /// @brief Currently-supported code pages in addition
   enum class TextEncoding : uint32_t
   {
      ANSI         = 0,       // aka CP_ACP
      WINDOWS_1250 = 1250,    // ANSI Central European; Central European (Windows)
      WINDOWS_1251 = 1251,    // ANSI Cyrillic; Cyrillic (Windows)
      WINDOWS_1252 = 1252,    // ANSI Latin 1; Western European (Windows)
      WINDOWS_1253 = 1253,    // ANSI Greek; Greek (Windows)
      WINDOWS_1254 = 1254,    // ANSI Turkish; Turkish (Windows)
      WINDOWS_1255 = 1255,    // ANSI Hebrew; Hebrew (Windows)
      WINDOWS_1256 = 1256,    // ANSI Arabic; Arabic (Windows)
      WINDOWS_1257 = 1257,    // ANSI Baltic; Baltic (Windows)
      WINDOWS_1258 = 1258,    // ANSI/OEM Vietnamese; Vietnamese (Windows)
      ISO_8859_1   = 28591,   // ISO 8859-1 Latin 1; Western European (ISO)
      ISO_8859_2   = 28592,   // ISO 8859-2 Central European; Central European (ISO)
      ISO_8859_3   = 28593,   // ISO 8859-3 Latin 3
      ISO_8859_4   = 28594,   // ISO 8859-4 Baltic
      ISO_8859_5   = 28595,   // ISO 8859-5 Cyrillic
      ISO_8859_6   = 28596,   // ISO 8859-6 Arabic
      ISO_8859_7   = 28597,   // ISO 8859-7 Greek
      ISO_8859_8   = 28598,   // ISO 8859-8 Hebrew; Hebrew (ISO-Visual)
      ISO_8859_9   = 28599,   // ISO 8859-9 Turkish
      ISO_8859_13  = 28603,   // ISO 8859-13 Estonian
      ISO_8859_15  = 28605,   // ISO 8859-15 Latin 9
      UTF8         = 65001,
   };


   /// @brief gets the charset name for a given codepage
   /// @param codepage
   /// @return
   auto codepageToCharset(TextEncoding codepage) -> std::optional<std::string_view>;

   /// @brief gets the codepage for a given character set name (case-sensitive)
   /// @param charset - name of the character set (e.g. "windows-1252")
   /// @return the codepage value, if a match was found, std::nullopt otherwise
   auto charsetToCodepage(std::string_view charset) -> std::optional<TextEncoding>;

   /// @brief gets the codepage for a given character set name (case-insensitive)
   /// @param charset - name of the character set (e.g. "windows-1252")
   /// @return the codepage value, if a match was found, std::nullopt otherwise
   auto charsetToCodepageNoCase(std::string charset) -> std::optional<TextEncoding>;

   /// @brief convert text to UTF8 from other narrow/multi-byte encoding.
   /// @return the converted string if successful, std::nullopt if not.
   ///
   [[nodiscard]] auto toUTF8(const std::string& text, TextEncoding from_code_page) -> MaybeString;


   /// @brief convert text to UTF8 from other narrow/multi-byte encoding.
   /// @return the converted string if successful, std::nullopt if not.
   ///
   [[nodiscard]] auto fromUTF8(const std::string& utf8_text, TextEncoding to_code_page) -> MaybeString;


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
      std::string result{ std::forward<Str>(text) };
      tryExpandEnvironmentVars(result);
      return result;
   }


   /// @brief Parses a text value into a nullable boolean.
   /// @param text Case-insensitive input to parse. Recognized true values: "true", "1", "yes"; recognized false values: "false", "0", "no".
   /// @return NullableBool containing true or false when the input matches a recognized value, otherwise std::nullopt to indicate an unknown/invalid boolean.
   inline auto textToBool(std::string_view text) -> NullableBool
   {
      constexpr auto TRUE_STR  = "true";
      constexpr auto FALSE_STR = "false";
      constexpr auto ONE_STR   = "1";
      constexpr auto ZERO_STR  = "0";
      constexpr auto YES_STR   = "yes";
      constexpr auto NO_STR    = "no";

      if (boost::iequals(text, TRUE_STR) || boost::iequals(text, ONE_STR) || boost::iequals(text, YES_STR)) return true;

      if (boost::iequals(text, FALSE_STR) || boost::iequals(text, ZERO_STR) || boost::iequals(text, NO_STR)) return false;

      return std::nullopt;
   }

   /// @brief create a folder path on disk including any missing parent folders.
   ///
   /// safe to call if folder already exists.
   auto createFolderPath(const fs::path& folder) noexcept -> bool;


   // A modern C++23 trim function for std::string_view
   [[nodiscard]] constexpr std::string_view trim(std::string_view sv, std::string_view whitespace = " \t\n\r\f\v") noexcept
   {
      const auto start = sv.find_first_not_of(whitespace);
      if (start == std::string_view::npos)
      {
         return {};   // Entire string is whitespace
      }

      const auto end = sv.find_last_not_of(whitespace);
      return sv.substr(start, end - start + 1);
   }


   [[nodiscard]] Buffer base64Decode(std::string_view encoded_str);

}   // namespace ctb
