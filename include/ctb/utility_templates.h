#pragma once

#include "ctb/ctb.h"
#include <magic_enum/magic_enum.hpp>

#include <charconv>
#include <optional>

namespace ctb
{

   /// @brief a functor object that is overloaded for multiple types 
   ///
   template <typename... Ts>
   struct Overloaded : Ts...
   {
      using Ts::operator()...;
   };
   template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;


   /// @brief  user-friendly version of from_chars that works with string_view and string
   /// @return an optional containing the requested value if successful, or an empty optional otherwise.
   /// 
   template<typename T, StringViewCompatibleType S>
   std::optional<T> from_str(S str)
   {
      T val{};
      auto result = std::from_chars(str.data(), str.data() + str.size(), val);

      if (result.ec != std::errc())
         return std::nullopt;  // there was an error, so return null

      return val;
   }


   /// @brief helper function to convert a zero-based index to the corresponding enum value, since the syntax is so fugly
   ///
   template<typename Enum>
   constexpr Enum enumFromIndex(int idx)
   {
      if (static_cast<size_t>(idx) >= magic_enum::enum_count<Enum>())
         assert("Invalid enum index, this is a bug." and false);

      return magic_enum::enum_value<Enum>(static_cast<size_t>(idx));
   }


   /// @brief convert a property enum into its zero-based index
   /// 
   template<typename Enum>
   constexpr int enumToIndex(Enum enum_val)
   {
      auto maybe_idx = magic_enum::enum_index(enum_val);
      if (!maybe_idx)
         assert(false);

      return static_cast<int>(*maybe_idx);
   }


} // namespace ctb