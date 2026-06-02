#pragma once

#include "App.h"
#include "controls/ElasticPropertyValueBase.h"

#include <wx/sizer.h>

namespace ctb::app
{
   /// @brief this class manages a property row in a WineDetailsBasePanel-derived panel, and handles showing/hiding
   ///        the row based on whether or not the property exists in the dataset.
   ///
   class WineDetailPanelRow
   {
   public:
      static constexpr auto COL_COUNT = 2;

      WineDetailPanelRow(wxSizer* parent_sizer, ElasticPropertyValueBase* value_ctrl, std::string_view heading_label)
         : m_parent_sizer{ parent_sizer }, m_value_ctrl{ value_ctrl }
      {
         auto* parent_ctrl = value_ctrl ? value_ctrl->GetParent() : nullptr;

         if (!parent_ctrl or !value_ctrl) throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };

         m_row_sizer  = new wxGridSizer{ COL_COUNT };                                         // cppcheck-suppress noOperatorEq
         m_label_ctrl = new wxStaticText{ parent_ctrl, wxID_ANY, wxFromSV(heading_label) };   // cppcheck-suppress noOperatorEq

         m_row_sizer->Add(m_label_ctrl, wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT).Right());
         m_row_sizer->Add(m_value_ctrl, wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT));
         parent_sizer->Add(m_row_sizer, wxSizerFlags{}.CenterHorizontal());
      }

      /// @brief Show/hide this row based on dataset contents
      ///
      /// If the bound control doesn't have any data, the control will be hidden. This mean that null properties will not be shown
      /// unless they have a display value other than empty string.
      void updateVisibility()
      {
         assert(m_value_ctrl);
         show(m_value_ctrl->hasDisplayValue());
      }

      /// @brief Force this detail row to show/hide itself
      /// @param show  if true the row will be set to visible, if false it will be hidden.
      void show(bool show = true)
      {
         assert(m_parent_sizer);
         m_parent_sizer->Show(m_row_sizer, show, true);
      }

   private:
      wxSizer*                  m_parent_sizer{};
      wxSizer*                  m_row_sizer{};
      wxStaticText*             m_label_ctrl{};
      ElasticPropertyValueBase* m_value_ctrl{};
   };

}   // namespace ctb::app
