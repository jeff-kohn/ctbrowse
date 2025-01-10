///
/// @file Error.h
///
/// @brief declaration for the cts::Error class
///
#pragma once

#include "magic_enum/magic_enum.hpp"

#include <exception>
#include <string>
#include <string_view>


namespace cts
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
         Generic,
         CurlError,
         HttpStatus,
      };


      /// @brief numeric error code, 0 indicates success, -1 indicates general/unknown failure, other numbers can
      ///        be used for contextual error codes.
      int64_t error_code{};


      /// @brief text description of the error that occurred, defaults to empty string if not specified.
      std::string error_message{};


      /// @brief the category of error this object represents
      Category category{ Category::Generic };


      /// @brief  the textual name of the Error::Category
      std::string_view categoryName() const
      {
         return magic_enum::enum_name(category);
      }


      /// @brief base class override, returns same value as message()
      const char* what() const noexcept override
      {
         return error_message.c_str();
      }


      /// @brief construct an Error with numeric error code, textual error message, and category
      Error(int64_t code, std::string error_message, Category category = Category::Generic) noexcept :
         error_code{ static_cast<int64_t>(code) },
         error_message{ std::move(error_message) },
         category{ category }
      {}


      /// @brief construct an Error with message and optional category
      explicit Error(std::string error_message, Category category = Category::Generic) noexcept :
         error_code{ ERROR_CODE_GENERAL_FAILURE },
         error_message{ std::move(error_message) },
         category{ category }

      {}

      Error() = default;
      Error(const Error&) = default;
      Error(Error&&) = default;
      Error& operator=(const Error&) = default;
      Error& operator=(Error&&) = default;
      ~Error() override = default;
   };

} // namespace cts
