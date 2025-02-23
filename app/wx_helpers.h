/*********************************************************************
 * @file       wx_helpers.cpp
 *
 * @brief      some helper functions for working with wxWidgets types
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/concepts.h"
#include "ctb/utility.h"

#include <wx/activityindicator.h>


namespace ctb::app
{

   /// @brief convert a range of strings/string_views to a wxArrayString
   ///
   template <rng::input_range Rng> requires StringViewCompatible<rng::range_value_t<Rng> >
   wxArrayString wxToArrayString(Rng&& strings)
   {
      Overloaded overloaded{
         [](std::string&& str)
         {
            return wxString{ str };
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


   /// @brief just a convenience wrapper for converting a string_view to a wxString
   ///
   inline wxString wxFromSV(std::string_view sv)
   {
      return wxString{sv.data(), sv.size() };
   }


   /// @brief  small object that sets a frame window's status text on destruction
   ///
   template <typename Wnd>
   struct ScopedStatusText
   {
      std::string message{};
      Wnd*        target{};

      ScopedStatusText(std::string_view msg, Wnd* target) : message{ msg }, target{ target } {}
      ~ScopedStatusText()
      {
         if (target)
            target->SetStatusText(message);
      }

      ScopedStatusText() = default;
      ScopedStatusText(const ScopedStatusText&) = default;
      ScopedStatusText(ScopedStatusText&&) = default;
      ScopedStatusText& operator=(const ScopedStatusText&) = default;
      ScopedStatusText& operator=(ScopedStatusText&&) = default;
   };

   template<class Wnd> ScopedStatusText(std::string_view, Wnd*) -> ScopedStatusText<Wnd>;


} // namespace ctb::app
