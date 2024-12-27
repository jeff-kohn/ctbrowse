#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace ctwin
{
   namespace fs = std::filesystem;

   /// <summary>
   ///   retrieve an environment variable. If the environment variable is not
   ///   found, or an error occurs, default_val will be returned. Otherwise the
   ///   variable's value (which could be an empty string) will be returned. Max
   ///   value length returned is MAX_ENV_VAR_LENGTH
   /// </summary>
   std::string getEnvironmentVar(const char* var_name, std::string_view default_val = "");

   inline std::string getEnvironmentVar(const std::string& var_name, std::string_view default_val = "")
   {
     return getEnvironmentVar(var_name.c_str(), default_val);
   }


   /// <summary>
   ///   expand any environment variables in the supplied string, replacing them with their values.
   /// </summary>
   /// <param name="text"></param>
   /// <returns></returns>
   bool expandEnvironmentVars(std::string& text);


   /// <summary>
   ///   just dump some text to a file.
   /// </summary>
   bool saveTextToFile(std::string_view text, fs::path file_path, bool overwrite = true) noexcept;

}  // namespace ctwin
