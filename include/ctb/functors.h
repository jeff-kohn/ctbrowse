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


   /// @brief helper function to convert a zero-based index to the corresponding enum value, since the syntax is so fugly
   ///
   template<typename Enum>
   constexpr Enum enumFromIndex(int idx)
   {
      using namespace magic_enum;

      if (static_cast<size_t>(idx) >= enum_count<Enum>())
         assert("Invalid enum index, this is a bug.");

      return enum_value<Enum>(static_cast<size_t>(idx));
   }


   /// @brief convert a property enum into its zero-based index
   /// 
   template<typename Enum>
   constexpr int enumToIndex(Enum enum_val)
   {
      using namespace magic_enum;
      auto index = enum_index(enum_val);
      return static_cast<int>(*index);
   }

} // namespace ctb

