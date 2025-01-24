/*******************************************************************
 * @file GridTableWineList.cpp
 *
 * @brief source file for the girdtableWineList class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#include "grids/GridTableWineList.h"
#include "grids/CellarTrackerGrid.h"

#include "ctb/functors.h"

#include <magic_enum/magic_enum.hpp>
#include <cassert>
#include <memory>


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

      if (row >= std::ssize(m_data)) // don't use GetNumerRows because it's virtual
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

      return m_data[row_idx][prop]
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
      auto attr_ptrrov_p = GetAttrProvider();
      assert(attr_ptrrov_p);

      for (const auto& [idx, disp_col] : vws::enumerate(m_display_columns))
      {
         auto attr_ptr = attr_ptrrov_p->GetAttrPtr(0, idx, wxGridCellAttr::wxAttrKind::Col);
         if (!attr_ptr)
            attr_ptr.reset(default_attr_ptr->Clone());

         attr_ptr->SetAlignment(std::to_underlying(disp_col.col_align), wxALIGN_CENTRE);
         attr_ptrrov_p->SetColAttr(attr_ptr.release(), idx);
      }
   }




} // namespace ctb