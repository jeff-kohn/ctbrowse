/*********************************************************************
 * @file       Error.h
 *
 * @brief      Declaration for the exception class Error
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/constants.h"
#include "ctb/ctb_format.h"
#include <magic_enum/magic_enum.hpp>

#include <exception>
#include <string>
#include <string_view>


#ifdef __WIN32 
   #undef Error
#endif

namespace ctb
{
   /// @brief exception class used for runtime errors
   ///
   /// This exception class supports error categories, numeric error codes and error text.  A default-constructed
   /// instance of this class will have a Category of Success with an error code of 0.
   /// 
   class Error final : public std::exception
   {
   public:
      static inline constexpr int64_t ERROR_CODE_GENERAL_FAILURE = -1;

      /// @brief enum for categorizing errors. may be useful for determining context for error_code value.
      enum class Category
      {
         ArgumentError,
         CurlError,
         DataError,
         FileError,
         GenericError,
         HttpStatus,
         OperationCanceled,
         ParseError,
         NotSupported,
         UiError
      };

      /// @brief numeric error code, 0 indicates success, -1 indicates general/unknown failure, other numbers can
      ///        be used for contextual error codes.
      int64_t error_code{};

      /// @brief text description of the error that occurred, defaults to empty string if not specified.
      std::string error_message{};

      /// @brief the category of error this object represents
      Category category{ Category::GenericError };

      /// @brief  the textual name of the Error::Category
      std::string_view categoryName() const
      {
         return magic_enum::enum_name(category);
      }

      /// @brief formattedMessage()
      /// @return a formatted error message combining the properties of this object.
      /// 
      std::string formattedMesage() const
      {
         return ctb::format("{} ({} {})", what(), categoryName(), error_code);
      }

      /// @brief base class override
      ///
      /// returns same value as error_message member variable, NOT formattedMessage()
      const char* what() const noexcept override
      {
         return error_message.c_str();
      }

      /// @brief construct an Error with numeric error code, textual error message, and category
      Error(int64_t code, std::string error_message, Category category = Category::GenericError) noexcept :
         error_code{ static_cast<int64_t>(code) },
         error_message{ std::move(error_message) },
         category{ category }
      {}

      /// @brief construct an Error with message and optional category
      explicit Error(std::string error_message, Category category = Category::GenericError) noexcept :
         error_code{ ERROR_CODE_GENERAL_FAILURE },
         error_message{ std::move(error_message) },
         category{ category }
      {}

      /// @brief construct an error with the given category and formatted message
      template <typename... T>
      Error(Category category, std::string_view fmt, T&&... args) : 
         error_code{ ERROR_CODE_GENERAL_FAILURE },
         error_message{ ctb::vformat(fmt, ctb::make_format_args(args...)) },
         category{ category }
      {}

      /// @brief construct an error with the given error code, category and formatted message
      template <typename... T>
      Error(int64_t code, Category category, std::string_view fmt, T&&... args) :
         error_code{ code },
         error_message{ ctb::vformat(fmt, ctb::make_format_args(args...)) },
         category{ category }
      {}

      Error() = default;
      Error(const Error&) = default;
      Error(Error&&) = default;
      Error& operator=(const Error&) = default;
      Error& operator=(Error&&) = default;
      ~Error() override = default;
   };


   /// @brief tranlate an exception_ptr to a ctb::Error
   /// 
   /// useful for code that wants to handle all exceptions with a
   /// catch(...) and return them as an unexpected{ Error{} }
   /// 
   inline auto packageError(std::exception_ptr ep = std::current_exception() ) noexcept -> Error
   {
      try 
      {
         if (ep) std::rethrow_exception(ep);
      }
      catch (ctb::Error e)      { return e;                 }
      catch (std::exception e)  { return Error{ e.what() }; }
      catch (...)
      {
         assert("wtf, nonstandard exception caught." and false);
      }

      return Error{ constants::ERROR_STR_UNKNOWN };
   }

} // namespace ctb
