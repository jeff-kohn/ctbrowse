#pragma once

#include <string>

namespace ctwin
{
   /// <summary>
   ///   retrieve an environment variable. If the environment variable is not
   ///   found, or an error occurs, default_val will be returned. Otherwise the
   ///   variable's value (which could be an empty string) will be returned. Max
   ///   value length returned is MAX_ENV_VAR_LENGTH
   /// </summary>
   std::string getEnvironmentVariable(const char* var_name,
                                      std::string_view default_val = "");

   inline std::string getEnvironmentVariable(const std::string& var_name,
                                             std::string_view default_val = "")
   {
     return getEnvironmentVariable(var_name.c_str(), default_val);
   }

}  // namespace ctwin