#include "ctb/utility.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <frozen/map.h>

#include <fstream>
#include <limits>

#if defined(_WIN32_WINNT)
   #include <Windows.h>
#endif

namespace ctb
{
   namespace
   {
      // MSVS adds third ifstream param to specify the file sharing flags, which we use if available.
      template<typename FileStream = std::fstream>
      auto openFile(const fs::path& file_path, std::ios_base::openmode mode, int share_flag) -> FileStream
      {

#if defined(_WIN32_WINNT)
         return FileStream{ file_path, mode, share_flag };
#else
         return FileStream{ file_path, mode };
#endif
      }

   }   // namespace


   auto readBinaryFile(const fs::path& file_path, uint32_t max_size) noexcept(false) -> Buffer
   {
      constexpr auto max_stream = std::numeric_limits<std::streamsize>::max();

      auto file = openFile<std::ifstream>(file_path, std::ios_base::binary, _SH_DENYWR);
      if (!file) throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_OPEN_FAILED, file_path.generic_string() };

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
      file.read(reinterpret_cast<char*>(buf.data()), std::ssize(buf));   // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      if (file.fail())
      {
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_READ_FAILED, file_path.generic_string() };
      }
      file.close();
      buf.resize(static_cast<size_t>(file.gcount()));
      return buf;
   }


   auto saveBinaryFile(const fs::path& file_path, BufferSpan buf, bool overwrite) noexcept(false) -> void
   {
      using std::ios_base;

      if (fs::exists(file_path) && !overwrite)
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_ALREADY_EXISTS, file_path.generic_string() };

      std::ofstream file{ file_path, ios_base::binary | ios_base::trunc | ios_base::out | ios_base::noreplace };
      if (!file) throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_OPEN_FAILED, file_path.generic_string() };

      file.write(reinterpret_cast<char*>(buf.data()), std::ssize(buf));   // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      if (file.fail())
      {
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_READ_FAILED, file_path.generic_string() };
      }
      file.close();
   }


   auto saveTextToFile(const fs::path& file_path, std::string_view text, bool overwrite) noexcept(false) -> void
   {
      if (fs::exists(file_path) && !overwrite)
         throw Error{ Error::Category::FileError, constants::FMT_ERROR_FILE_ALREADY_EXISTS, file_path.generic_string() };

      if (file_path.has_parent_path()) fs::create_directories(file_path.parent_path());

      // use binary mode to keep ofstream from inserting extra carriage returns, since
      // we want to preserve whatever line feeds are already in the file
      auto file_out = openFile<std::ofstream>(file_path, std::ios_base::out | std::ios_base::binary, _SH_DENYRW);
      file_out.write(text.data(), std::ssize(text));   // NOLINT(bugprone-suspicious-stringview-data-usage)
   }


   auto viewFilename(std::string& fq_path) noexcept -> std::string_view
   {
      // we use this view, so that we can call substr() to get another view,
      // whereas calling std::string::substr() would return a new string
      // instead of a view
      std::string_view path{ fq_path };

      auto sep = path.find_last_of('\\');
      if (std::string_view::npos == sep) sep = path.find_last_of('/');

      if (std::string_view::npos == sep)
         return path;
      else
         return path.substr(sep + 1);
   }


   auto createFolderPath(const fs::path& folder) noexcept -> bool
   {
      // MS in their infinite wisdom, will return false even though the directory was created if
      // the string had a trailing slash, so we have to ignore return value and check for an error_code
      std::error_code ms_sucks{};
      fs::create_directories(folder, ms_sucks);
      if (ms_sucks)
      {
         SPDLOG_DEBUG("createFolderPath() failed for '{}'. {}", folder.generic_string(), ms_sucks.message());
         return false;
      }
      return true;
   }


#if defined(_WIN32_WINNT)

   auto tryExpandEnvironmentVars(std::string& text) -> bool
   {
      constexpr int padding = 2;   //The API requires a buffer equal to string length + '\0' + 1

      // Find out how big of a string we need to accommodate.
      auto bufsize = ExpandEnvironmentStrings(text.c_str(), nullptr, 0);
      if (0 == bufsize or bufsize <= text.length() + padding) return false;

      std::vector<char> dest(bufsize, 0);
      if (ExpandEnvironmentStrings(text.c_str(), dest.data(), bufsize))
      {
         // TODO: might be nice to get rid of this copy but would have to deal with extra trailing nulls in dest
         text = dest.data();
         return true;
      }
      return false;
   }


   [[nodiscard]] auto toUTF8(const std::string& text, TextEncoding from_code_page) -> MaybeString
   {
      auto code_page = to_underlying(from_code_page);

      int length = MultiByteToWideChar(code_page, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, text.c_str(), -1, nullptr, 0);
      if (!length) return {};

      std::vector<wchar_t> wide_buf(static_cast<size_t>(length), '\0');
      if (!MultiByteToWideChar(
             code_page, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, text.c_str(), -1, wide_buf.data(), static_cast<int>(wide_buf.size())))
      {
         return {};
      }

      // Get needed buffer length since some UTF-16 chars may need multiple bytes in UTF-8.
      length = WideCharToMultiByte(
         CP_UTF8, WC_COMPOSITECHECK | WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS, wide_buf.data(), -1, nullptr, 0, nullptr, nullptr);
      if (!length) return {};

      // Now allocate buffer and make the final call to do the conversion.
      std::vector<char> utf8_buf(static_cast<size_t>(length), '\0');
      if (WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK | WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS, wide_buf.data(), -1,
                              utf8_buf.data(), static_cast<int>(utf8_buf.size()), nullptr, nullptr))
         return std::string{ utf8_buf.data() };

      return {};
   }


   [[nodiscard]] auto fromUTF8(const std::string& utf8_text, TextEncoding to_code_page) -> MaybeString
   {
      MaybeString result{};
      auto        code_page = to_underlying(to_code_page);

      // First convert UTF-8 to UTF-16
      int length = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8_text.c_str(), -1, nullptr, 0);
      if (!length) return result;

      std::vector<wchar_t> wide_buf(static_cast<size_t>(length), '\0');
      if (!MultiByteToWideChar(
             CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8_text.c_str(), -1, wide_buf.data(), static_cast<int>(wide_buf.size())))
         return result;

      // Get needed buffer length for the target code page then do the conversion
      length = WideCharToMultiByte(code_page, WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS, wide_buf.data(), -1, nullptr, 0, nullptr, nullptr);
      if (length)
      {
         std::vector<char> mb_buf(static_cast<size_t>(length), '\0');
         length = WideCharToMultiByte(code_page, WC_COMPOSITECHECK | WC_NO_BEST_FIT_CHARS, wide_buf.data(), -1, mb_buf.data(),
                                      static_cast<int>(mb_buf.size()), nullptr, nullptr);
         if (length)
         {
            // Since utf8_text.data() and utf8_text.size() to pass the string to MultiByteToWideChar, it gets treated as non-null-terminated and we have to terminate result string.
            std::string retval{ mb_buf.data() };
            retval.push_back('\0');
            result = retval;
         }
      }

      return result;
   }

#else
   // TODO: provide implementation for other platforms when needed.
#endif


   static inline constexpr auto codepage_map = frozen::make_map<std::string_view, TextEncoding>({
      { "ansi",         TextEncoding::ANSI         },
      { "windows-1250", TextEncoding::WINDOWS_1250 },
      { "windows-1251", TextEncoding::WINDOWS_1251 },
      { "windows-1252", TextEncoding::WINDOWS_1252 },
      { "windows-1253", TextEncoding::WINDOWS_1253 },
      { "windows-1254", TextEncoding::WINDOWS_1254 },
      { "windows-1255", TextEncoding::WINDOWS_1255 },
      { "windows-1256", TextEncoding::WINDOWS_1256 },
      { "windows-1257", TextEncoding::WINDOWS_1257 },
      { "windows-1258", TextEncoding::WINDOWS_1258 },
      { "utf-8",        TextEncoding::UTF8         },
      { "utf8",         TextEncoding::UTF8         },
      { "iso-8859-1",   TextEncoding::ISO_8859_1   },
      { "iso-8859-2",   TextEncoding::ISO_8859_2   },
      { "iso-8859-3",   TextEncoding::ISO_8859_3   },
      { "iso-8859-4",   TextEncoding::ISO_8859_4   },
      { "iso-8859-5",   TextEncoding::ISO_8859_5   },
      { "iso-8859-6",   TextEncoding::ISO_8859_6   },
      { "iso-8859-7",   TextEncoding::ISO_8859_7   },
      { "iso-8859-8",   TextEncoding::ISO_8859_8   },
      { "iso-8859-9",   TextEncoding::ISO_8859_9   },
      { "iso-8859-13",  TextEncoding::ISO_8859_13  },
      { "iso-8859-15",  TextEncoding::ISO_8859_15  },
   });


   auto charsetToCodepage(std::string_view charset) -> std::optional<TextEncoding>
   {
      const auto* iter = codepage_map.find(charset);
      if (iter == codepage_map.end()) return {};
      return iter->second;
   }


   auto charsetToCodepageNoCase(std::string charset) -> std::optional<TextEncoding>
   {
      boost::to_lower(charset);
      const auto* iter = codepage_map.find(charset);
      if (iter == codepage_map.end()) return {};
      return iter->second;
   }

#pragma warning(push)
#pragma warning(disable : 4702)   // MSVC bug there's no unrachable code.

   auto codepageToCharset(TextEncoding codepage) -> std::optional<std::string_view>
   {
      // there are two entries for  CP_UTF8 so we handle as special case to be consistent in what we return.
      if (codepage == TextEncoding::UTF8) return "utf-8";

      // just do a manual search, there aren't that many and it's a cheap comparison.
      auto matches = codepage_map |
                     vws::filter(
                        [codepage](const auto& elem)
                        {
                           return codepage == elem.second;
                        }) |
                     vws::keys;

      for (auto match : matches)
      {
         return match;
      }
      return {};
   }

#pragma warning(pop)

}   // namespace ctb
