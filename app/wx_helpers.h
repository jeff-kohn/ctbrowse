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
#include "ctb/utility_templates.h"

#include <wx/arrstr.h>
#include <wx/config.h>


namespace ctb::app
{

   /// @brief just a convenience wrapper for converting a string_view to a wxString
   ///
   inline wxString wxFromSV(std::string_view sv)
   {
      return wxString{ sv.data(), sv.size() };
   }

   /// @brief returns a string_view on a wxString
   /// 
   /// the string_view is only valid for the lifetime of the wx_string it views
   /// 
   //inline std::string_view wxViewString(const wxString& wx_string)
   //{
   //   const auto buf = wx_string.utf8_str();
   //   return std::string_view{ buf.data(), buf.length()};
   //}


   /// @brief convert a range of strings/string_views to a wxArrayString
   ///
   template <rng::input_range Rng> requires StringOrStringViewType<rng::range_value_t<Rng> >
   wxArrayString wxToArrayString(Rng&& strings)
   {
      Overloaded overloaded{
         [](std::string&& str)
         {
            return wxString{ str };
         },
         [] (std::string_view sv)
         {
            return wxFromSV(sv);
         },
         [] (auto&& str)
         {
            return wxString{ std::forward<decltype(str)>(str) };
         }
      };

      return std::forward<decltype(strings)>(strings) | vws::transform(overloaded)
                                                      | rng::to<wxArrayString>();
   }


   /// @brief small object that sets a frame window's status text on destruction
   ///
   template <typename WndT>
   struct ScopedStatusText
   {
      std::string message{};
      WndT*        target{};

      ScopedStatusText(std::string_view msg, WndT* target) : message{ msg }, target{ target } {}
      ~ScopedStatusText()
      {
         if (target)
            target->SetStatusText(message);
      }

      ScopedStatusText() noexcept = default;
      ScopedStatusText(const ScopedStatusText&) = default;
      ScopedStatusText(ScopedStatusText&&) = default;
      ScopedStatusText& operator=(const ScopedStatusText&) = default;
      ScopedStatusText& operator=(ScopedStatusText&&) = default;
   };
   // deduction guide
   template<typename WndT> ScopedStatusText(std::string_view, WndT*) -> ScopedStatusText<WndT>;


   /// @brief class to restore config object to root path when going out of scope
   ///
   /// this is useful because the config object's current path is persistent and some
   /// of the wxWidgets code assumes starting path of "/" (eg wxPersist functionality)
   /// 
   class ScopedConfigPath final
   {
   public:
      static inline constexpr const char* CONFIG_ROOT = "/";

      explicit ScopedConfigPath(wxConfigBase& config) : m_config(config)
      {}
      ~ScopedConfigPath()        { m_config.SetPath(CONFIG_ROOT); }
      
      wxConfigBase& get()        { return m_config;       }
      wxConfigBase* operator->() { return &m_config;      }
      wxConfigBase& operator*()  { return m_config;       }

      ScopedConfigPath() = delete;

   private:
      wxConfigBase& m_config;
   };


} // namespace ctb::app
