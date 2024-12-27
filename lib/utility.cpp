#include "ctwin/utility.h"
#include "ctwin/constants.h"

#include "windows_includes.h"

#include <fstream>
#include <string>
#include <vector>

namespace ctwin
{

//#if defined(_WIN32)

   /// <summary>
   ///   retrieve an environment variable. If the environment variable is not found, or an
   ///   error occurs, default_val will be returned. Otherwise the variable's value
   ///   (which could be an empty string) will be returned. Max value length returned
   ///   is MAX_ENV_VAR_LENGTH, value will be truncted if it's longer than that.
   /// </summary>
   std::string getEnvironmentVar(const char* var_name, std::string_view default_val)
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



   bool expandEnvironmentVars(std::string& text)
   {
      // Find out how big of a string we need to accommodate
      auto bufsize = ExpandEnvironmentStrings(text.c_str(), nullptr, 0);
      if (0 == bufsize)
         return false;

      // When using ANSI strings, the buffer size should be the string length,
      // plus terminating null character, plus one.
      std::string dest(bufsize + 2, '\0');

      if (ExpandEnvironmentStrings(text.c_str(), dest.data(), bufsize))
      {
         text.swap(dest);
         return true;
      }
      else
         return false;
   }

// #endif  // _WIN32


   /// <summary>
   ///   just dump some text to a file.
   /// </summary>
   bool saveTextToFile(std::string_view text, fs::path file_path, bool overwrite) noexcept
   {
      if (fs::exists(file_path) && !overwrite)
         return false;

      if (file_path.has_parent_path())
         fs::create_directories(file_path.parent_path());

      std::ofstream file_out{ file_path };
      file_out << text;
      return true;
   }


} // namespace ctwin
