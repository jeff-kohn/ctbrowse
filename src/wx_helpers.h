/*********************************************************************
 * @file       wx_helpers.cpp
 *
 * @brief      some helper functions for working with wxWidgets types
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "concepts.h"


namespace cts
{

   /// @brief a functor object that is overloaded for multiple types 
   template < typename... Ts >
   struct Overloaded : Ts...
   {
      using Ts:: operator()...;
   };


   /// @brief convert a range of strings/string_views to a wxArrayString
   template <rng::input_range Rng> requires StringViewCompatible<rng::range_value_t<Rng> >
   wxArrayString wxToArrayString(Rng&& strings)
   {
      Overloaded overloaded{
         [](std::string&& str)
         {
            return wxString{ std::move(str) };
         },
         [] (std::string_view sv)
         {
            return wxString{ sv.data(), sv.length() };
         },
         [] (auto&& str)
         {
            return wxString{ std::forward<decltype(str)>(str) };
         }
      };

      return std::forward<decltype(strings)>(strings) | vws::transform(overloaded)
                                                      | rng::to<wxArrayString>();
   }

} // namespace cts
