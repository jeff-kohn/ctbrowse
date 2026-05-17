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
   template<typename T>
   std::optional<T> from_str(std::string_view str)
   {
      T val{};
      auto result = std::from_chars(str.data(), str.data() + str.size(), val);

      if (result.ec != std::errc())
         return std::nullopt;  // there was an error, so return null

      return val;
   }
   
} // namespace ctb