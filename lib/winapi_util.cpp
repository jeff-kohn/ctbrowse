/*********************************************************************
 * @file       winapi_util.cpp
 *
 * @brief      helper functions for using the WinAPI
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/winapi_util.h"
#include "ctb/constants.h"

#include <cpr/curlholder.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include <windows.h>
#include <shlwapi.h>
#include <WinInet.h>


namespace ctb::util
{
   std::string percentEncode(std::string_view text)
   {
      // this just makes sure that curl's global init has been called, we don't actually need a handle
      cpr::CurlHolder holder;

      char* output = curl_easy_escape(nullptr, text.data(), static_cast<int>(text.length()));
      if (output)
      {
         std::string result = output;
         curl_free(output);
         return result;
      }
      return "";
   }

   std::string percentDecode(std::string_view text)
   {
      // this just makes sure that curl's global init has been called, we don't actually need a handle
      cpr::CurlHolder holder; 

      char* output = curl_easy_escape(nullptr, text.data(), static_cast<int>(text.length()));
      if (output)
      {
         std::string result = output;
         curl_free(output);
         return result;
      }
      return "";
   }


   std::string toUTF8(const std::string& text, unsigned int code_page)
   {
      int length = MultiByteToWideChar(code_page, MB_PRECOMPOSED, text.c_str(), -1, nullptr, 0);
      if (!length)
         return "";

      std::vector<wchar_t> wide_buf(static_cast<size_t>(length), '\0');
      if (!MultiByteToWideChar(code_page, MB_PRECOMPOSED, text.c_str(), -1, wide_buf.data(), static_cast<int>(wide_buf.size())))
         return "";

      // Get needed buffer length since some UTF-16 chars may need multiple bytes in UTF-8. 
      length = WideCharToMultiByte(CP_UTF8, 0, wide_buf.data(), -1, nullptr, 0, nullptr, nullptr);
      if (!length)
         return "";

      // Now allocate buffer and make the final call to do the conversion.
      std::vector<char> utf8_buf(static_cast<size_t>(length), '\0');
      if (WideCharToMultiByte(CP_UTF8, 0, wide_buf.data(), -1, utf8_buf.data(), static_cast<int>(utf8_buf.size()), nullptr, nullptr))
          return std::string{ utf8_buf.data() };

      return "";
   }


   std::string getEnvironmentVar(const char* var_name, std::string_view default_val)
   {
      if (nullptr == var_name || *var_name == '\0')
         return std::string(default_val);

      // std::getenv() isn't thread safe so use the WinAPI.
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


   bool saveTextToFile(std::string_view text, fs::path file_path, bool overwrite) noexcept
   {
      if (fs::exists(file_path) && !overwrite)
         return false;

      if (file_path.has_parent_path())
         fs::create_directories(file_path.parent_path());

      // use binary mode to keep ofstream from inserting extra carriage returns, since
      // we want to preserve whatever line feeds are already in the file (it may already
      // have CR/LF, in which case we'd end up with extra CR in text mode because
      // ofstream isn't smart enough to recognize it).
      std::ofstream file_out{ file_path, std::ios_base::out | std::ios_base::binary };
      file_out << text;
      return true;
   }


} // namespace ctb::util
