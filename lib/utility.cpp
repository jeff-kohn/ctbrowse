#include "ctb/utility.h"

#include <fstream>
#include <limits>

#if defined(_WIN32_WINNT)
   #include <Windows.h>
   #include <Shlwapi.h>
   #include <wininet.h>
#endif

namespace ctb
{
   namespace
   {
      // MSVS adds third ifstream param to specify the file sharing flags, which we use if available.
      //
      template <typename FileStream  = std::fstream>
      auto openFile(const fs::path& file_path, std::ios_base::openmode mode,  int share_flag) -> FileStream
      {

      #if defined(_WIN32_WINNT)
         return FileStream{ file_path, mode, share_flag };
      #else
         return FileStream{ file_path, mode };
      #endif

      }
   } // namespace


   void readBinaryFile(const fs::path& file_path, std::vector<char>& buf, uint32_t max_size) noexcept(false)
   {
      constexpr auto max_stream = std::numeric_limits<std::streamsize>::max();

      auto file = openFile<std::ifstream>(file_path, std::ios_base::binary,  _SH_DENYWR);
      if (!file)
         throw Error{ Error::Category::FileError, "Couldn't open file {},", file_path.generic_string() };

      // get size
      file.ignore(max_stream);
      auto file_size = static_cast<size_t>(file.gcount());
      file.clear();   //  Since ignore will have set eof.
      file.seekg(0, std::ios_base::beg);

      if (file_size > max_size)
      {
         throw Error{
            Error::Category::FileError,
            "File {}'s size of {} bytes exceeded the maximum allowable size of {}.",
            file_size, file_path.filename().generic_string(), max_size
         };
      }

      buf.resize(file_size);
      file.read(buf.data(), std::ssize(buf));
      buf.resize(static_cast<size_t>(file.gcount()));
   }


   void saveTextToFile(std::string_view text, fs::path file_path, bool overwrite) noexcept(false)
   {
      if (fs::exists(file_path) && !overwrite)
         throw Error{ Error::Category::FileError, "File {} already exists and will not be overwritten.", file_path.generic_string() };

      if (file_path.has_parent_path())
         fs::create_directories(file_path.parent_path());

      // use binary mode to keep ofstream from inserting extra carriage returns, since
      // we want to preserve whatever line feeds are already in the file
      auto file_out = openFile<std::ofstream>(file_path, std::ios_base::out | std::ios_base::binary, _SH_DENYRW); 
      file_out << text;
   }


#if defined(_WIN32_WINNT)

   bool tryExpandEnvironmentVars(std::string& text)
   {
      constexpr int padding = 2; //The API requires a buffer equal to string length + '\0' + 1

      // Find out how big of a string we need to accommodate. 
      auto bufsize = ExpandEnvironmentStrings(text.c_str(), nullptr, 0);
      if (0 == bufsize or bufsize <= text.length() + padding)
         return false;

      std::vector<char> dest(bufsize, 0);
      if (ExpandEnvironmentStrings(text.c_str(), dest.data(), bufsize))
      {
         text = dest.data();
         return true;
      }
      return false;
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

#else
   // TODO: provie implementation for other platforms when needed.
#endif


} // namespace ctb