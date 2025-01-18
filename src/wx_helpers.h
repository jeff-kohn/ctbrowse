/*********************************************************************
 * @file       wx_helpers.cpp
 *
 * @brief      some helper functions for working with wxWidgets types
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "concepts.h"

#include "cts/functors.h"
#include <wx/activityindicator.h>
#include <chrono>

namespace cts
{

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

   /// @brief  small object that sets a frame window's status text on destruction
   template <typename Wnd>
   struct ScopedStatusText
   {
      std::string message{};
      Wnd*        target_ptr{};

      ScopedStatusText() = default;
      ScopedStatusText(std::string_view msg, Wnd* target) : message{ msg }, target_ptr{ target } {}
      ~ScopedStatusText()
      {
         if (target_ptr)
            target_ptr->SetStatusText(message);
      }
   };


} // namespace cts
