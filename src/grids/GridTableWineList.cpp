/*******************************************************************
 * @file GridTableWineList.cpp
 *
 * @brief source file for the GriTableWineList class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#include "grids/GridTableWineList.h"
#include "grids/CellarTrackerGrid.h"

#include "ctb/functors.h"

#include <magic_enum/magic_enum.hpp>
#include <wx/font.h>

#include <cassert>
#include <memory>
#include <vector>


namespace ctb
{
   using data::WineListData;
   using data::WineListEntry;


   wxString GridTableWineList::GetColLabelValue(int col)
   {
      auto col_idx = static_cast<size_t>(col);  // size_t just makes everything ugly, I hate it

      if (col_idx >= m_display_columns.size())
      {
         /// If we get here we got a request for invalid column, should be impossible...
         assert(false);
         return std::format("Col {}", col);
      }
      return wxString{ m_display_columns[col_idx].display_name };
   }


   wxString GridTableWineList::GetValue(int row, int col)
   {
      using namespace ctb::data;

      if (row >= std::ssize(*m_view)) // don't use GetNumerRows because it's virtual
      {
         assert(false); 
         return wxString{};
      }
      if (col >= std::ssize(m_display_columns))
      {
         assert(false);
         return wxString{};
      }

      auto row_idx  = static_cast<size_t>(row);
      auto display_col = m_display_columns[static_cast<size_t>(col)];
      auto prop = display_col.prop_id;

      // if the returned expected<> has a value then we retrieve it and return it to caller
      // otherwise we just return an empty string
      return (*m_view)[row_idx][prop]
         .and_then([&display_col](WineListEntry::ValueWrapper val) -> std::expected<wxString, Error>
            {
               auto val_str = display_col.getDisplayValue(val);
               return wxString{ val_str.data(), val_str.size() };
            })
         .value_or(wxString{});
   }


   void GridTableWineList::SetValue(int row, int col, const wxString& value) 
   {
      throw Error{constants::ERROR_EDITING_NOT_SUPPORTED };
   }


   void GridTableWineList::configureGridColumns(wxGridCellAttrPtr default_attr_ptr)
   {
      auto attr_prov_ptr = GetAttrProvider();
      assert(attr_prov_ptr);

      for (const auto& [idx, disp_col] : vws::enumerate(m_display_columns))
      {
         // update existing attr if it exists, otherwise use a copy of the default attr
         auto attr_ptr = attr_prov_ptr->GetAttrPtr(0, idx, wxGridCellAttr::wxAttrKind::Col);
         if (!attr_ptr)
            attr_ptr.reset(default_attr_ptr->Clone());

         if (idx == 0)
         {
            auto font = attr_ptr->GetFont();
            font.SetWeight(wxFontWeight::wxFONTWEIGHT_SEMIBOLD);
            attr_ptr->SetFont(font);
         }
         attr_ptr->SetAlignment(std::to_underlying(disp_col.col_align), wxALIGN_CENTRE);
         attr_prov_ptr->SetColAttr(attr_ptr.release(), idx); // transfer ownership
      }

      // Reset 0,0 attribute so it gets column attr, rather than original 
      // default that was created by CellarTrackerGrid::GetOrCreateCellAttrPtr()
      attr_prov_ptr->SetAttr(nullptr, 0, 0);
   }


   void GridTableWineList::filterBySubstring(std::string_view substr)
   {
      auto cols = getDisplayColumns() | vws::transform([](const DisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
                                      | rng::to<std::vector>();

      m_substring_filter = SubstringFilter{ std::string{substr}, cols };
      refreshView();
   }


   void GridTableWineList::filterBySubstring(std::string_view substr, size_t col_idx)
   {
      SubstringFilter::Prop prop = magic_enum::enum_value<SubstringFilter::Prop>(col_idx);
      m_substring_filter = SubstringFilter{ std::string{substr},  std::vector<SubstringFilter::Prop>{prop} };
      refreshView();      
   }


   void GridTableWineList::clearSubstringFilter()
   {
      m_substring_filter = std::nullopt;
      refreshView();
   }


   void GridTableWineList::refreshView()
   {
      // first, check for any match filters...


      // second, check for any substring filters
      if (m_substring_filter)
      {

         auto cols = getDisplayColumns() | vws::transform([](const DisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
                                         | rng::to<std::vector>();

         auto filtered_data = vws::all(m_data) | vws::filter(*m_substring_filter)
                                               | rng::to<std::deque>();
         m_filtered_data.swap(filtered_data);
         m_view = &m_filtered_data;
      }
   }


} // namespace ctb