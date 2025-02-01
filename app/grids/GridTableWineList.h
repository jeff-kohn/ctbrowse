/*******************************************************************
 * @file GridTableWineList.h
 *
 * @brief Header file for the class GridTableWineList
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "app_constants.h"
#include "ctb/data/DisplayColumn.h"
#include "ctb/data/SubStringFilter.h"
#include "ctb/data/WineListEntry.h"
#include "grids/GridTableBase.h"

#include <magic_enum/magic_enum.hpp>
#include <wx/grid.h>

#include <array>
#include <vector>


namespace ctb::app
{

   /// @brief GridTable-derived class used with CellarTrackerGrid to display table data from CellarTracker.com
   ///
   /// This class is NOT THREADSAFE at the instance level. This shouldn't be a problem since this class is meant
   /// to be used from UI code (specifically CellarTrackerGrid). 
   /// 
   class GridTableWineList : public GridTableBase
   {
   public:
      /// @brief type used for describing how to display a column in the grid
      using DisplayColumn = data::DisplayColumn<data::WineListEntry>;
      using SubStringFilter = data::SubStringFilter<data::WineListEntry>;

      explicit GridTableWineList(data::WineListData&& data) : 
         m_data{std::move(data)},
         m_view{&m_data},
         m_display_columns(DefaultDisplayColumns.begin(), DefaultDisplayColumns.end())
      {}

      static inline const std::array DefaultDisplayColumns { 
         DisplayColumn{ data::WineListEntry::Prop::WineAndVintage,                              constants::LBL_WINE     },
         DisplayColumn{ data::WineListEntry::Prop::Country                                                              },
         DisplayColumn{ data::WineListEntry::Prop::Region                                                               },
         DisplayColumn{ data::WineListEntry::Prop::Appellation                                                          },
         //DisplayColumn{ data::WineListEntry::Prop::Quantity,   DisplayColumn::Format::Number,   constants::LBL_QTY      },
         //DisplayColumn{ data::WineListEntry::Prop::Pending,    DisplayColumn::Format::Number                            },
         //DisplayColumn{ data::WineListEntry::Prop::CTScore,    DisplayColumn::Format::Decimal,  constants::LBL_CT_SCORE },
         //DisplayColumn{ data::WineListEntry::Prop::MYScore,    DisplayColumn::Format::Decimal,  constants::LBL_MY_SCORE },
         //DisplayColumn{ data::WineListEntry::Prop::Price,      DisplayColumn::Format::Currency, constants::LBL_MY_PRICE },
         //DisplayColumn{ data::WineListEntry::Prop::EndConsume, DisplayColumn::Format::Number,   constants::LBL_DRINK_BY }
      };


      /// @brief base class override that returns the number of rows/records in the table/grid
      int GetNumberRows() override { return std::ssize(*m_view); }


      /// @brief base class override that returns the number of columns displayed in the table/grid
      int GetNumberCols() override { return std::ssize(m_display_columns); }


      /// @brief base class override to get the display name for column headers
      wxString GetColLabelValue(int col) override;


      /// @brief base class override to get the value for a given cell in the grid
      wxString GetValue(int row, int col) override;


      /// @brief this function will always throw an exception since the grid is read-only
      void SetValue(int row, int col, const wxString& value) override;


      /// @brief container alias used to hold the DisplayColumn's used for this grid
      using ColumnList = std::vector<DisplayColumn>;


      /// @brief  get a list of the columns that will be displayed in the grid
      /// 
      /// the columns are in the order they will be displayed.
      ColumnList getDisplayColumns() const { return m_display_columns; }


      /// @brief specify the columns to display in the grid
      template<rng::input_range Cols> requires std::is_same_v<rng::range_value_t<Cols>, ColumnList::value_type>
      void setDisplayColumns(Cols&& cols) 
      {
         ColumnList new_cols{std::forward<Cols>(cols)};
         m_display_columns.swap(new_cols);
      }


      /// @brief this method will configure the column alignment settings for the grid based on the
      ///        settings in the DisplayColumn objects.
      void configureGridColumns(wxGridCellAttrPtr default_attr) override;


      /// @brief filter the table by performing a substring search across all columns
      ///
      /// note that class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      bool filterBySubstring(std::string_view substr) override;


      /// @brief filter the table by performing a substring search on the specified column
      ///
      /// note that class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      bool filterBySubstring(std::string_view substr, size_t col_idx) override;


      /// @brief clear/reset the substring filter
      void clearSubStringFilter() override;


      size_t getTotalRowCount() const override       {  return m_data.size();  }  
      size_t getFilteredRowCount() const override    {  return m_view->size(); }


      // this class is meant to be instantiated on the heap.
      GridTableWineList() = delete;
      GridTableWineList(const GridTableWineList&) = delete;
      GridTableWineList(GridTableWineList&&) = delete;
      GridTableWineList& operator=(const GridTableWineList&) = delete;
      GridTableWineList& operator=(GridTableWineList&&) = delete;

   private:
      data::WineListData             m_data{};
      data::WineListData             m_filtered_data{};  // due to mechanics of wxGrid, we need to copy the dataset when filtering
      data::WineListData*            m_view{};           // may point to m_data or m_filtered_data depending if filter is active
      ColumnList                     m_display_columns{};
      std::optional<SubStringFilter> m_substring_filter{};

      bool filterBySubstringImpl(std::string_view substr, const SubStringFilter& filter);
      void refreshView();
   };


} // namespace ctb::app