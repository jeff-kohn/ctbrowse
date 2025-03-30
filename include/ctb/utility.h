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
   /// @return true if the file was read, false if not.
   /// 
   /// @throws ctb::Error, possibly other std::exception-derived if file can't be read or larger than max_size
   /// 
   void readBinaryFile(const fs::path& file_path, std::vector<char>& buf, uint32_t max_size = constants::ONE_MB) noexcept(false);


   /// @brief just dump some text to a file (no encoding or formatting applied).
   /// @throws ctb::Error, possibly other std::exception-derived if file can't be written or already exists
   ///         and overwrite = false;
   ///
   /// if the file_path has a parent directory and it doesn't exist, an attempt will be made to 
   /// create it. 
   /// 
   /// 
   void saveTextToFile(std::string_view text, fs::path file_path, bool overwrite = false) noexcept(false);


   /// @brief  Expand environment variables in place
   /// @return true if successful, false if unsuccessful in which case the parameter 'text' will be unmodified
   /// 
   /// in the case when the passed string does not contain any environment vars, this function
   /// returns without any allocation or copying, which gives it a slight performance edge over
   /// expandEnvironmentVars() if you're working with strings that may or may not contain any vars.
   /// 
   bool tryExpandEnvironmentVars(std::string& text);


   /// @brief convert text to UTF8 from other narrow/multi-byte encoding.
   ///
   /// see https://learn.microsoft.com/en-us/windows/win32/Intl/code-page-identifiers
   /// for a list of code page id's, 
   /// 
   std::string toUTF8(const std::string& text, unsigned int code_page = 28591 /* ISO 8859-1 Latin 1; Western European (ISO) */);


   /// @brief expand environment variables and return result
   ///
   /// while this overload is convenient, it has the overhead of an unnecessary copy
   /// when the passed string has no vars to expand.
   inline std::string expandEnvironmentVars(std::string_view text)
   {
      std::string result{};
      tryExpandEnvironmentVars(result);
      return result;
   }


   /// @brief get view/substring of just the filename from a string_view containing a path
   ///
   inline std::string_view fileNamePart(std::string_view fq_path) noexcept
   {
      auto sep = fq_path.find_last_of('\\');
      if (std::string_view::npos == sep)
         sep = fq_path.find_last_of('/');

      if (std::string_view::npos == sep)
         return fq_path;
      else
         return fq_path.substr(sep + 1);
   }



} // namespace ctb