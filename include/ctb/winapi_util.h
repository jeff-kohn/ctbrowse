/*********************************************************************
 * @file       winapi_util.h
 *
 * @brief      Declaration for helper functions for using the WinAPI
 * 
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace ctb::util
{
   namespace fs = std::filesystem;

   /// @brief  percent-encode a string to make it compatible with HTTP requests
   /// @return the converted string, or an empty string if the conversion failed.
   std::string percentEncode(std::string_view text);


   /// @brief decode a percent-encoded string
   /// @return the converted string, or an empty string if the conversion failed.
   std::string percentDecode(std::string_view text);


   /// @brief convert text to UTF8 from other narrow/multi-byte encoding.
   ///
   /// see https://learn.microsoft.com/en-us/windows/win32/Intl/code-page-identifiers
   /// for a list of code page id's, 
   /// 
   std::string toUTF8(const std::string& text, unsigned int code_page = 28591 /* ISO 8859-1 Latin 1; Western European (ISO) */);


   /// @brief just dump some text to a file.
   ///
   /// if the file_path has a parent directory and it doesn't exist, an attempt will be made to 
   /// create it. 
   /// 
   /// returns true on success; returns false if the file save operation fails or the parent folder 
   /// doesn't exist and can't be created.
   /// 
   bool saveTextToFile(std::string_view text, fs::path file_path, bool overwrite = true) noexcept;

}  // namespace ctb::util
