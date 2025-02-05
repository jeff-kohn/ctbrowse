/*******************************************************************
 * @file GridTableWineList.cpp
 *
 * @brief source file for the GriTableWineList class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#include "grid/GridTableWineList.h"
#include "grid/CellarTrackerGrid.h"

#include "ctb/functors.h"

#include <magic_enum/magic_enum.hpp>
#include <wx/font.h>

#include <cassert>
#include <memory>
#include <vector>


namespace ctb::app
{
   using data::WineListData;
   using data::WineListEntry;


   [[nodiscard]] GridTablePtr GridTableWineList::create(data::WineListData&& data)
   {
      return std::shared_ptr<GridTableWineList>{ new GridTableWineList{ std::move(data) } };  
   }


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

      // if the expected value is returned, format it as string and return it to caller
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


   void GridTableWineList::configureGridColumns(wxGridCellAttrPtr default_attr)
   {
      auto attr_prov = GetAttrProvider();
      assert(attr_prov);

      for (const auto& [idx, disp_col] : vws::enumerate(m_display_columns))
      {
         // update existing attr if it exists, otherwise use a copy of the default attr
         auto attr = attr_prov->GetAttrPtr(0, idx, wxGridCellAttr::wxAttrKind::Col);
         if (!attr)
            attr.reset(default_attr->Clone());

         if (idx == 0)
         {
            auto font = attr->GetFont();
            font.SetWeight(wxFontWeight::wxFONTWEIGHT_SEMIBOLD);
            attr->SetFont(font);
         }
         attr->SetAlignment(std::to_underlying(disp_col.col_align), wxALIGN_CENTRE);
         attr_prov->SetColAttr(attr.release(), idx); // transfer ownership
      }

      // Reset 0,0 attribute so it gets column attr, rather than original 
      // default that was created by CellarTrackerGrid::GetOrCreateCellAttrPtr()
      attr_prov->SetAttr(nullptr, 0, 0);
   }


   bool GridTableWineList::filterBySubstring(std::string_view substr)
   {
      auto cols = getDisplayColumns() | vws::transform([](const DisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
                                      | rng::to<std::vector>();

      return filterBySubstringImpl(substr, SubStringFilter{ std::string{substr}, cols });
   }


   bool GridTableWineList::filterBySubstring(std::string_view substr, size_t col_idx)
   {
      SubStringFilter::Prop prop = magic_enum::enum_value<SubStringFilter::Prop>(col_idx);
      return filterBySubstringImpl(substr, SubStringFilter{ std::string{substr},  std::vector<SubStringFilter::Prop>{prop} });
   }


   void GridTableWineList::clearSubStringFilter()
   {
      m_substring_filter = std::nullopt;
      m_view = &m_data;
   }


   std::vector<IGridTable::SortOptionName> GridTableWineList::availableSortOptions() const
   {
      std::vector<IGridTable::SortOptionName> options{};
      options.reserve(GridTableWineList::SortOptions.size()); 

      for (const auto& [i, table_sort] : vws::enumerate(GridTableWineList::SortOptions))
      {
         options.emplace_back(IGridTable::SortOptionName{ static_cast<size_t>(i), table_sort.sort_name  });
      }
      return options;
   }


   ctb::app::IGridTable::SortOptionName GridTableWineList::currentSortSelection() const
   {
      return SortSelection{ m_sort_index };
   }


   void GridTableWineList::setSortSelection(size_t index) 
   {
      m_sort_index = index;
      sortData();
   }



   bool GridTableWineList::filterBySubstringImpl(std::string_view substr, const SubStringFilter& filter)
   {
      auto filtered_data = vws::all(m_data) | vws::filter(filter)
                                            | rng::to<std::deque>();

      // we only update the grid if there is some matching data
      if (filtered_data.empty())
         return false;

      m_substring_filter = filter;
      m_filtered_data.swap(filtered_data);
      m_view = &m_filtered_data;

      return true;
   }


   void GridTableWineList::sortData()
   {
      // sort the data table, then re-apply any filters to the view. Otherwise we'd have to sort twice
      rng::sort(m_data, SortOptions[m_sort_index]);

      // todo
   }


} // namespace ctb::app