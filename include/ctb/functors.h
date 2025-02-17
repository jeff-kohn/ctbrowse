/*******************************************************************
 * @file functors.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include <magic_enum/magic_enum.hpp>

namespace ctb
{

   /// @brief a functor object that is overloaded for multiple types 
   template < typename... Ts >
   struct Overloaded : Ts...
   {
      using Ts:: operator()...;
   };



} // namespace ctb


  /// @brief helper function to convert a zero-based index to the corresponding enum value, since the syntax is so fugly
  ///
template<typename Enum>
constexpr Enum enumFromIndex(int idx)
{
   if (static_cast<size_t>(idx) >= magic_enum::enum_count<Enum>())
      assert("Invalid enum index, this is a bug.");

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

