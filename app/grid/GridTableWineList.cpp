/*******************************************************************
 * @file GridTableWineList.cpp
 *
 * @brief source file for the GriTableWineList class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "grid/GridTableWineList.h"

#include <ctb/utility.h>
#include <magic_enum/magic_enum.hpp>
#include <wx/font.h>

#include <cassert>
#include <memory>
#include <vector>


namespace ctb::app
{

   [[nodiscard]] GridTablePtr GridTableWineList::create(WineListData&& data)
   {
      return std::shared_ptr<GridTableWineList>{ new GridTableWineList{ std::move(data) } };  
   }


   wxString GridTableWineList::GetColLabelValue(int col)
   {
      auto col_idx = static_cast<size_t>(col);  // size_t just makes everything ugly, I hate it

      if (col_idx >= m_display_columns.size())
      {
         /// If we get here we got a request for invalid column, i.e. a bug
         assert(false);
         return std::format("Col {}", col);
      }
      return wxString{ m_display_columns[col_idx].display_name };

   }


   wxString GridTableWineList::GetValue(int row, int col)
   {
      try
      {
         if (row >= std::ssize(*m_current_view)) // don't use GetNumberRows because it's virtual
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

         // format as string and return it to caller
         auto val = (*m_current_view)[row_idx][prop];
         auto val_str = display_col.getDisplayValue(val);
         return wxString{ val_str.data(), val_str.size() };
      }
      catch(std::exception&)
      {
         // don't display an error message here because if there's a problem with the data in a row or column,
         // user might get dozens (or hundreds) of messages.
         // TODO: LOGGING
      }

      // if we get here it's likely a bug, shouldn't happen.
      return constants::ERROR_VAL;
   }


   void GridTableWineList::SetValue(int, int, const wxString&) 
   {
      throw Error{constants::ERROR_STR_EDITING_NOT_SUPPORTED };
   }


   void GridTableWineList::configureGridColumns(wxGridCellAttrPtr default_attr)
   {
      auto attr_prov = GetAttrProvider();
      assert(attr_prov);

      for (const auto&& [idx, disp_col] : vws::enumerate(m_display_columns))
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
      // this overload searches all columns in the current grid, so get the prop_id's 
      auto cols = getDisplayColumns() | vws::transform([](const DisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
                                      | rng::to<std::vector>();

      return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
   }


   bool GridTableWineList::filterBySubstring(std::string_view substr, int col_idx)
   {
      auto prop = RecordType::Traits::propFromIndex(col_idx);
      auto cols = std::vector{ prop };

      return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
   }


   void GridTableWineList::clearSubStringFilter()
   {
      m_substring_filter = std::nullopt;
      applyPropFilters();
   }


   std::vector<GridTableWineList::GridTableSortConfig>  GridTableWineList::availableSortConfigs() const
   {
      std::vector<GridTableSortConfig> configs{};
      configs.reserve(GridTableWineList::Sorters.size()); 

      for (const auto&& [i, table_sort] : vws::enumerate(GridTableWineList::Sorters))
      {
         configs.emplace_back(GridTableSortConfig{ static_cast<int>(i), table_sort.sort_name  });
      }
      return configs;
   }


   GridTableWineList::GridTableSortConfig  GridTableWineList::activeSortConfig() const 
   {
      return m_sort_config;
   }


   void GridTableWineList::applySortConfig(const GridTableSortConfig& config)
   {
      if (config != m_sort_config)
      {
         m_sort_config = config;
         sortData();
      }
   }


   std::vector<GridTableFilter> GridTableWineList::availableFilters() const
   {
      return std::vector<GridTableFilter>(std::begin(StringFilters), std::end(StringFilters));
   }


   StringSet GridTableWineList::getFilterMatchValues(int prop_idx) const
   {
      return PropFilterMgrString::getFilterMatchValues(m_grid_data, RecordType::Traits::propFromIndex(prop_idx));
   }


   bool GridTableWineList::addFilter(int prop_idx, std::string_view value)
   {
      // if we somehow get passed a filter we already have, don't waste our time.
      if ( m_prop_filters.addFilter(RecordType::Traits::propFromIndex(prop_idx), value) )
      {
         applyPropFilters();
         return true;
      }
      return false;
   }


   bool GridTableWineList::removeFilter(int prop_idx, std::string_view match_value)
   {
      // if we somehow get passed filter that we aren't using, don't waste our time.
      if ( m_prop_filters.removeFilter(RecordType::Traits::propFromIndex(prop_idx), match_value) )
      {
         applyPropFilters();
         return true;
      }
      return false;
   }


   // some helper functions for composing filters to apply to the grid_data.
   namespace 
   {
      template<rng::input_range RngT> requires std::same_as<std::decay_t<rng::range_value_t<RngT> >, GridTableWineList::RecordType>
      auto applyPropStringFilters(RngT&& rng, GridTableWineList::PropFilterMgrString& filters)
      {
         return vws::all(std::forward<RngT>(rng)) | vws::filter([&filters](const GridTableWineList::RecordType& rec) {  return filters.isMatch(rec); });
      }


      template<rng::input_range RngT> requires std::same_as<std::decay_t<rng::range_value_t<RngT> >, GridTableWineList::RecordType>
      auto applyInStockFilter(RngT&& rng, const GridTableWineList::GreaterThanFilter& filter)
      {
         return vws::all(std::forward<RngT>(rng)) | vws::filter([&filter](const GridTableWineList::RecordType& rec) {  return filter(rec); });
      }
   

      template<rng::input_range RngT> requires std::same_as<std::decay_t<rng::range_value_t<RngT> >, GridTableWineList::RecordType>
      auto applyStringSearchFilter(RngT&& rng, const GridTableWineList::SubStringFilter& filter)
      {
         return vws::all(std::forward<RngT>(rng)) | vws::filter([&filter](const GridTableWineList::RecordType& rec) {  return filter(rec); });
      }
   } // namespace 



   void GridTableWineList::applyPropFilters()
   {
      if (m_prop_filters.activeFilters() and m_in_stock_filter)
      {
         m_filtered_data =  applyInStockFilter(applyPropStringFilters(m_grid_data, m_prop_filters), *m_in_stock_filter) | rng::to<TableType>();
         m_current_view = &m_filtered_data;
      }
      else if (m_prop_filters.activeFilters())
      {
         m_filtered_data = applyPropStringFilters(m_grid_data, m_prop_filters) | rng::to<TableType>();
         m_current_view = &m_filtered_data;
      }
      else if (m_in_stock_filter)
      {
         m_filtered_data = applyInStockFilter(m_grid_data, *m_in_stock_filter)| rng::to<TableType>();
         m_current_view = &m_filtered_data;
      }
      else{
         m_current_view = &m_grid_data;
         m_filtered_data = WineListData{};
      }
   }


   bool GridTableWineList::applySubStringFilter(const SubStringFilter& filter)
   {
      auto filtered = applyStringSearchFilter(*m_current_view, filter) | rng::to<TableType>();

      if (filtered.empty())
         return false;
    
      m_substring_filter = filter;
      m_filtered_data.swap(filtered);
      m_current_view = &m_filtered_data;
      return true;
   }


   void GridTableWineList::sortData()
   {
      // sort the data table, then re-apply any filters to the view. Otherwise we'd have to sort twice
      if (m_sort_config.ascending)
      {
         rng::sort(m_grid_data, Sorters[static_cast<size_t>(m_sort_config.sort_index)]);
      }
      else
      {
         rng::sort(vws::reverse(m_grid_data), Sorters[static_cast<size_t>(m_sort_config.sort_index)]);
      }

      applyPropFilters();
      if (m_substring_filter)
      {
         applySubStringFilter(*m_substring_filter);
      }
   }

   bool GridTableWineList::enableInStockFilter(bool enable)
   {
      if (enable)
         m_in_stock_filter = GreaterThanFilter{ PropId::Quantity, uint16_t{0} }; // data type has to be exact match.
      else
         m_in_stock_filter = std::nullopt;
      
      applyPropFilters();
      return true;
   }


   const CtProperty& GridTableWineList::getDetailProp(int row_idx, std::string_view prop_name)
   {
      auto maybe_prop = magic_enum::enum_cast<PropId>(prop_name);
      if (!maybe_prop)
         return null_prop; // can't return default-constructed becuase it would be ref to temp

      return (*m_current_view)[static_cast<size_t>(row_idx)][*maybe_prop];
   }


} // namespace ctb::app