#include "ctwin/environment.h"
#include "ctwin/constants.h"

#include "windows_includes.h"

#include <string>
#include <vector>

namespace ctwin
{
   /// <summary>
   ///   retrieve an environment variable. If the environment variable is not found, or an
   ///   error occurs, default_val will be returned. Otherwise the variable's value
   ///   (which could be an empty string) will be returned. Max value length returned
   ///   is MAX_ENV_VAR_LENGTH, value will be truncted if it's longer than that.
   /// </summary>
   std::string getEnvironmentVariable(const char* var_name, std::string_view default_val)
   {
      if (nullptr == var_name || *var_name == '\0')
         return std::string(default_val);

      // std::getenv() is problematic on Windows so use the WinAPI.
      // try with a modestly sized static array first, if it's not big enough
      // we can re-try with a dynamically-sized array
      char buf[constants::MAX_ENV_VAR_LENGTH] = { '\0' };
      auto max_var_length = sizeof(buf) - 1;
      auto actual_var_length = ::GetEnvironmentVariable(var_name, buf, sizeof(buf));
      if (actual_var_length <= max_var_length)
      {
         return std::string{ buf };
      }
      else if (actual_var_length > max_var_length)
      {
         std::vector<char> dyn_buf(actual_var_length);
         if (::GetEnvironmentVariable(var_name, dyn_buf.data(), static_cast<DWORD>(dyn_buf.size())))
            return std::string{dyn_buf.data()};
      }

      return std::string{ default_val };
   }


} // namespace ctwin