/*******************************************************************
 * @file GridTable.h
 *
 * @brief Header file for the GridTable base class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "grid/GridTableFilter.h"

#include <ctb/CtProperty.h>
#include <wx/grid.h>

#include <set>
#include <memory>


namespace ctb::app
{


   /// @brief struct that contains name, index and direction of a sort option
   ///
   struct GridTableSortConfig
   {
      int               sort_index{};
      std::string_view  sort_name{};
      bool              ascending{ true };

      [[nodiscard]] std::strong_ordering operator<=>(const GridTableSortConfig&) const = default;
   };



   /// @brief our "data model", provides a base class interface for accessing CellarTracker data
   /// 
   /// users of this interface should gain access through 
   /// IGridTableEventSource::getTable() or using the ptr supplied when
   /// handling IGridTableEventSource::notify()
   /// 
   class GridTable : public wxGridStringTable
   {
   public:

      /// @brief Sets up the formatting options in the calling grid to match this grid table's fields
      ///
      virtual void configureGridColumns(wxGridCellAttrPtr default_attr) = 0;


      /// @brief filter does substring matching on ANY column in the table view
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers GridTableEvent::SubStringFilter
      ///
      virtual bool filterBySubstring(std::string_view substr) = 0;


      /// @brief sets filter that does substring matching on the specified column
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers GridTableEvent::SubStringFilter
      /// 
      virtual bool filterBySubstring(std::string_view substr, int col_idx) = 0;


      /// @brief clear the substring filter
      ///
      /// triggers GridTableEvent::SubStringFilter
      /// 
      virtual void clearSubStringFilter() = 0;


      /// @brief returns the total number of records in the underlying dataset
      ///
      virtual int totalRowCount() const = 0;


      /// @brief returns the number of records with filters applied.
      ///
      virtual int filteredRowCount() const  = 0;


      /// @brief retrieves list of available sortOptions, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      /// 
      virtual std::vector<GridTableSortConfig> availableSortConfigs() const = 0;


      /// @brief returns the currently active sort option
      ///
      virtual GridTableSortConfig activeSortConfig() const = 0;


      /// @brief specifies a new sort option, triggers GridTableEvent::Sort
      ///
      virtual void applySortConfig(const GridTableSortConfig& config) = 0;


      /// @brief retrieves a list of available filters for this table.
      ///
      virtual std::vector<GridTableFilter> availableStringFilters() const = 0;


      /// @brief get a list of values that can be used to filter on a column in the table
      ///
      virtual StringSet getFilterMatchValues(int prop_idx) const = 0;


      /// @brief adds a match value filter for the specified column.
      ///
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      /// 
      /// a record must match at least one match_value for each property that 
      /// has a filter to be considered a match.
      /// 
      virtual bool addPropFilterString(int prop_idx, std::string_view match_value) = 0;


      /// @brief removes a match value filter for the specified column.
      ///
      /// @return true if the filter was removed, false if it wasn't found
      /// 
      virtual bool removePropFilterString(int prop_idx, std::string_view match_value) = 0;


      /// @brief retrieve a property from the underlying table record (as opposed to a grid column)
      /// 
      /// @return the property value formatted as a string if found, std::nullopt if not found 
      ///
      /// Since we don't have table-neutral indices to use, this lookup has to be done by
      /// property name as a string that corresponds to correct enum. 
      /// 
      virtual const CtProperty& getDetailProp(int row_idx, std::string_view prop_name) = 0;


      /// @brief apples "in-stock only" filter to the data set, if supported.
      /// @return true if the filter was applied, false if it was not 
      /// 
      /// not all grid table support this filter, you can check by calling hasInStockmFilter()
      /// 
      virtual bool enableInStockFilter(bool enable) = 0;


      /// @brief indicates whether the grid table supports filtering to in-stock only 
      /// @return true if supported, false otherwise.
      /// 
      virtual bool hasInStockFilter() const
      {  return false; }


      /// @brief retrieves the minimum score filter value if active.
      /// 
      virtual NullableDouble getMinScoreFilter() const = 0;


      /// @brief set the minimum score filter
      ///
      virtual bool setMinScoreFilter(NullableDouble min_score = std::nullopt) = 0;


      /// @brief retrieve the iWineId value for every record in the dataset.
      /// 
      virtual std::vector<uint64_t> getWineIds() = 0;


      /// @brief getTableName()
      /// @return the name of the CT table this grid table represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      /// 
      virtual std::string_view getTableName() const = 0;


      /// @brief destructor
      ///
      ~GridTable() override
      {}
   };


   /// @brief the smart-ptr-to-base that's used to work with the GridTable interface 
   ///
   using GridTablePtr = std::shared_ptr<GridTable>;


}  // namespace ctb::app