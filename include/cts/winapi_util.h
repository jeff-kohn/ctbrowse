#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace cts::util
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


   /// @brief retrieve an environment variable.
   ///
   /// If the environment variable is not found, or an error occurs,
   /// default_val will be returned. Otherwise the variable's value
   /// (which could be an empty string) will be returned. Max value
   /// length returned is MAX_ENV_VAR_LENGTH
   /// 
   std::string getEnvironmentVar(const char* var_name, std::string_view default_val = "");

   inline std::string getEnvironmentVar(const std::string& var_name, std::string_view default_val = "")
   {
     return getEnvironmentVar(var_name.c_str(), default_val);
   }


   /// @brief  expand any environment variables in the supplied string, replacing them with their values.
   bool expandEnvironmentVars(std::string& text);


   /// @brief just dump some text to a file.
   bool saveTextToFile(std::string_view text, fs::path file_path, bool overwrite = true) noexcept;

}  // namespace cts::util
