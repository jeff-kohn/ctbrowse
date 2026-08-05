/*******************************************************************
* @file utility.h
*
* @brief Header file for some helper templates
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <charconv>
#include <optional>
#include <variant>

namespace ctb
{

   /// @brief a functor object that is overloaded for multiple types
   ///
   template<typename... Ts>
   struct Overloaded : Ts...
   {
      using Ts::operator()...;
   };
   template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;


   /// @brief  user-friendly version of from_chars that works with string_view and string
   /// @return an optional containing the requested value if successful, or an empty optional otherwise.
   ///
   template<typename T>
   std::optional<T> from_str(std::string_view str)
   {
      T    val{};
      auto result = std::from_chars(str.data(), str.data() + str.size(), val);   // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]


      if (result.ec != std::errc()) return std::nullopt;   // there was an error, so return null

      return val;
   }


   /// @brief Convenience wrapper to either return the value from an expected, or throw its error type if it doesn't have a value.
   /// @return value_type from the expected, if present
   /// @throw error_type from the expected, if present
   template<ExpectedType ExpectedT>
   auto getValueOrThrow(ExpectedT&& expected_value) noexcept(false)
   {
      using error_type = std::remove_cvref_t<ExpectedT>::error_type;

      if (expected_value) return std::forward<ExpectedT>(expected_value).value();

      throw error_type{ std::forward<ExpectedT>(expected_value).error() };
   }


   // convert a variant into its text representation. As long as all the types
   // contained in the variant are formattable,
   template<typename... Args>
   std::string asString(const std::variant<Args...>& vt)
   {
      return std::visit(
         [](auto&& arg)
         {
            return ctb::format("{}", arg);
         },
         vt);
   }


}   // namespace ctb
