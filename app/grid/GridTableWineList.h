/*******************************************************************
 * @file GridTableWineList.h
 *
 * @brief Header file for the class GridTableWineList
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "interfaces/GridTableEvent.h"

#include <ctb/data/DisplayColumn.h>
#include <ctb/data/SubStringFilter.h>
#include <ctb/data/TableSorter.h>
#include <ctb/data/WineListEntry.h>

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
   class GridTableWineList : public IGridTable
   {
   public:
      /// @brief type used for describing how to display a column in the grid
      using RecordType = data::WineListEntry;
      using Prop = RecordType::Prop;
      using DisplayColumn = data::DisplayColumn<RecordType>;
      using SubStringFilter = data::SubStringFilter<RecordType>;
      using TableSort = data::TableSorter<RecordType>;
      using SortConfig = IGridTable::SortConfig;


      /// @brief static factory method to create an instance of the GridTableWineList class
      /// 
      static [[nodiscard]] GridTablePtr create(data::WineListData&& data);


      /// @brief list of display columns that can be used for a grid.
      ///
      static inline const std::array DefaultDisplayColumns { 
         DisplayColumn{ Prop::WineAndVintage,                              constants::LBL_WINE     },
         DisplayColumn{ Prop::Locale,                                      constants::LBL_LOCALE   },
         DisplayColumn{ Prop::Quantity,   DisplayColumn::Format::Number,   constants::LBL_QTY      },
         DisplayColumn{ Prop::Pending,    DisplayColumn::Format::Number                            },
         DisplayColumn{ Prop::CTScore,    DisplayColumn::Format::Decimal,  constants::LBL_CT_SCORE },
         DisplayColumn{ Prop::MYScore,    DisplayColumn::Format::Decimal,  constants::LBL_MY_SCORE },
      };


      /// @brief the available sortings for this table.
      ///
      static inline const std::array Sorters{ 
         TableSort{ { Prop::WineName, Prop::Vintage                    }, constants::SORT_OPTION_WINE_VINTAGE },
         TableSort{ { Prop::Vintage,  Prop::WineName                   }, constants::SORT_OPTION_VINTAGE_WINE },
         TableSort{ { Prop::Locale,   Prop::WineName,    Prop::Vintage }, constants::SORT_OPTION_LOCALE_WINE  },
         TableSort{ { Prop::Region,   Prop::WineName,    Prop::Vintage }, constants::SORT_OPTION_REGION_WINE  },
      };


      /// @brief base class override that returns the number of rows/records in the table/grid
      ///
      int GetNumberRows() override { return std::ssize(*m_current_view); }


      /// @brief base class override that returns the number of columns displayed in the table/grid
      ///
      int GetNumberCols() override { return std::ssize(m_display_columns); }


      /// @brief base class override to get the display name for column headers
      ///
      wxString GetColLabelValue(int col) override;


      /// @brief base class override to get the value for a given cell in the grid
      ///
      wxString GetValue(int row, int col) override;


      /// @brief this function will always throw an exception since the grid is read-only
      ///
      void SetValue(int row, int col, const wxString& value) override;


      /// @brief container alias used to hold the DisplayColumn's used for this grid
      ///
      using ColumnList = std::vector<DisplayColumn>;


      /// @brief  get a list of the columns that will be displayed in the grid
      /// 
      /// the columns are in the order they will be displayed.
      ///
      ColumnList getDisplayColumns() const { return m_display_columns; }


      /// @brief specify the columns to display in the grid
      ///
      template<rng::input_range Cols> requires std::is_same_v<rng::range_value_t<Cols>, ColumnList::value_type>
      void setDisplayColumns(Cols&& cols) 
      {
         ColumnList new_cols{std::forward<Cols>(cols)};
         m_display_columns.swap(new_cols);
      }


      /// @brief this method will configure the column alignment settings for the grid based on the
      ///        settings in the DisplayColumn objects.
      ///
      void configureGridColumns(wxGridCellAttrPtr default_attr) override;


      /// @brief filter the table by performing a substring search across all columns
      ///
      /// note that class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      ///
      bool filterBySubstring(std::string_view substr) override;


      /// @brief filter the table by performing a substring search on the specified column
      ///
      /// note that class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      ///
      bool filterBySubstring(std::string_view substr, int col_idx) override;


      /// @brief clear/reset the substring filter
      ///
      void clearSubStringFilter() override;


      /// @brief get the total row count
      ///
      int totalRowCount() const override       
      {  return std::ssize(m_grid_data);  }  


      /// @brief get the filtered row count
      ///
      int filteredRowCount() const override    
      {  return std::ssize(*m_current_view); }


      // returns a collection of available sort options. 
      ///
      std::vector<SortConfig> availableSortOptions() const override;


      // retrieves the currently-active sort option
      ///
      SortConfig currentSortSelection() const override;


      // sets the currently-active sort option
      ///
      void setSortSelection(int index, bool ascending = true) override;

   private:
      data::WineListData             m_grid_data{};                 // the underlying data records for this table.
      data::WineListData             m_filtered_data{};        // due to mechanics of wxGrid, we need to copy the dataset when filtering
      data::WineListData*            m_current_view{};                 // may point to m_grid_data or m_filtered_data depending if filter is active
      ColumnList                     m_display_columns{};
      int                            m_sort_index{0};
      bool                           m_sort_ascending{true};
      std::optional<SubStringFilter> m_substring_filter{};

      /// @brief private constructor used by static create()
      GridTableWineList(data::WineListData&& data) : 
         m_grid_data{std::move(data)},
         m_current_view{&m_grid_data},
         m_display_columns(DefaultDisplayColumns.begin(), DefaultDisplayColumns.end())
      {}

      //void filterData();
      bool applySubStringFilter(const SubStringFilter& filter);
      void sortData();
      bool isFilterActive()   {  return m_current_view = &m_filtered_data; }


      // this class is meant to be instantiated on the heap.
      GridTableWineList() = delete;
      GridTableWineList(const GridTableWineList&) = delete;
      GridTableWineList(GridTableWineList&&) = delete;
      GridTableWineList& operator=(const GridTableWineList&) = delete;
      GridTableWineList& operator=(GridTableWineList&&) = delete;
   };

} // namespace ctb::app