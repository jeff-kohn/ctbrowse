/*********************************************************************
* @file       CategorizedControls.h
*
* @brief      Defines the CategorizedControls template class
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/

#pragma once

#include "App.h"

#include <wx/window.h>

#include <map>


namespace ctb::app
{
   /// @brief Class to allow showing/hiding sets of controls based on category enum.
   template<typename EnumT> requires std::is_enum_v<EnumT>
   class CategorizedControls
   {
   public:
      using Category = EnumT;

      /// @brief Show/Hide all controls associated with the specified category
      void showCategory(Category category, bool show)
      {
         auto [beg, end] = m_categorized_controls.equal_range(category);
         for (auto* ctrl : rng::subrange{ beg, end } | vws::values) 
         {
            if (show)
               ctrl->Show();
            else
               ctrl->Hide();
         }
      }

      /// @brief Associate a control with a category
      void addControlDependency(Category category, wxWindow* ctrl)
      {
         m_categorized_controls.emplace(category, ctrl);
      }

   private:
      std::multimap<Category, wxWindow*> m_categorized_controls{};
   };


} // namespace ctb::app