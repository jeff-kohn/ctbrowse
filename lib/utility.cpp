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


   auto readBinaryFile(const fs::path& file_path, uint32_t max_size) noexcept(false) -> Buffer
   {
      constexpr auto max_stream = std::numeric_limits<std::streamsize>::max();

      auto file = openFile<std::ifstream>(file_path, std::ios_base::binary,  _SH_DENYWR);
      if (!file)
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_OPEN_FAILED, file_path.generic_string() };

      // get (and validate) size
      file.ignore(max_stream);
      auto file_size = static_cast<size_t>(file.gcount());
      file.clear();   //  Since ignore will have set eof.
      file.seekg(0, std::ios_base::beg);
      if (file_size > max_size)
      {
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_TOO_BIG, file_size, file_path.generic_string(), max_size };
      }

      // read the data into appropriately buffer and return to caller
      Buffer buf(file_size);
      file.read(reinterpret_cast<char*>(buf.data()), std::ssize(buf));
      if (file.fail())
      {
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_READ_FAILED, file_path.generic_string() };
      }
      file.close();
      buf.resize(static_cast<size_t>(file.gcount())); 
      return buf;
   }


   auto saveBinaryFile(const fs::path& file_path, BufferSpan buf, bool overwrite) noexcept(false)-> void
   {
      using std::ios_base;

      if (fs::exists(file_path) && !overwrite)
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_ALREADY_EXISTS, file_path.generic_string() };

      std::ofstream file{ file_path, ios_base::binary | ios_base::trunc | ios_base::out | ios_base::noreplace };
      if (!file)
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_OPEN_FAILED, file_path.generic_string() };

      file.write(reinterpret_cast<char*>(buf.data()), std::ssize(buf));
      if (file.fail())
      {
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_READ_FAILED, file_path.generic_string() };
      }
      file.close();
   }


   auto saveTextToFile(fs::path file_path, std::string_view text, bool overwrite) noexcept(false) -> void
   {
      if (fs::exists(file_path) && !overwrite)
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_ALREADY_EXISTS, file_path.generic_string() };

      if (file_path.has_parent_path())
         fs::create_directories(file_path.parent_path());

      // use binary mode to keep ofstream from inserting extra carriage returns, since
      // we want to preserve whatever line feeds are already in the file
      auto file_out = openFile<std::ofstream>(file_path, std::ios_base::out | std::ios_base::binary, _SH_DENYRW); 
      file_out << text;
   }


   auto viewFilename(std::string& fq_path) noexcept -> std::string_view
   {
      // we use this view, so that we can call substr() to get another view,
      // whereas calling std::string::substr() would return a new string 
      // instead of a view
      std::string_view path{ fq_path };

      auto sep = path.find_last_of('\\');
      if (std::string_view::npos == sep)
         sep = path.find_last_of('/');

      if (std::string_view::npos == sep)
         return path;
      else
         return path.substr(sep + 1);
   }



#if defined(_WIN32_WINNT)

   auto tryExpandEnvironmentVars(std::string& text) -> bool
   {
      constexpr int padding = 2; //The API requires a buffer equal to string length + '\0' + 1

      // Find out how big of a string we need to accommodate. 
      auto bufsize = ExpandEnvironmentStrings(text.c_str(), nullptr, 0);
      if (0 == bufsize or bufsize <= text.length() + padding)
         return false;

      std::vector<char> dest(bufsize, 0);
      if (ExpandEnvironmentStrings(text.c_str(), dest.data(), bufsize))
      {
         // TODO: might be nice to get rid of this copy but would have to deal with extra trailing nulls in dest
         text = dest.data(); 
         return true;
      }
      return false;
   }


   [[nodiscard]] auto toUTF8(const std::string& text, unsigned int code_page) -> MaybeString
   {
      int length = MultiByteToWideChar(code_page, MB_PRECOMPOSED, text.c_str(), -1, nullptr, 0);
      if (!length)
         return {};

      std::vector<wchar_t> wide_buf(static_cast<size_t>(length), '\0');
      if (!MultiByteToWideChar(code_page, MB_PRECOMPOSED, text.c_str(), -1, wide_buf.data(), static_cast<int>(wide_buf.size())))
         return {};

      // Get needed buffer length since some UTF-16 chars may need multiple bytes in UTF-8. 
      length = WideCharToMultiByte(CP_UTF8, 0, wide_buf.data(), -1, nullptr, 0, nullptr, nullptr);
      if (!length)
         return {};

      // Now allocate buffer and make the final call to do the conversion.
      std::vector<char> utf8_buf(static_cast<size_t>(length), '\0');
      if (WideCharToMultiByte(CP_UTF8, 0, wide_buf.data(), -1, utf8_buf.data(), static_cast<int>(utf8_buf.size()), nullptr, nullptr))
         return std::string{ utf8_buf.data() };

      return {};
   }


#else
   // TODO: provie implementation for other platforms when needed.
#endif


} // namespace ctb