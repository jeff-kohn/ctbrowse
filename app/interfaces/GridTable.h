#/*******************************************************************
 * @file GridTable.h
 *
 * @brief Header file for the GridTable interface
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"

#include <wx/grid.h>

#include <set>
#include <memory>


namespace ctb::app
{
   struct IGridTableEventSource;
   using GridTableEventSourcePtr = std::shared_ptr<IGridTableEventSource>;


   /// @brief our "data model", provides an interface for accessing CellarTrcker data
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


      /// @brief contains a name and index of a sort option
      ///
      struct SortConfig
      {
         int               sort_index{};
         std::string_view  sort_name{};
         bool              ascending{ true };

         [[nodiscard]] std::strong_ordering operator<=>(const SortConfig&) const = default;
      };


      /// @brief retrieves list of available sortOptions, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      /// 
      virtual std::vector<SortConfig> availableSortConfigs() const = 0;


      /// @brief returns the currently active sort option
      ///
      virtual SortConfig activeSortConfig() const = 0;


      /// @brief specifies a new sort option, triggers GridTableEvent::Sort
      ///
      virtual void setActiveSortConfig(const SortConfig& config) = 0;


      /// @brief specifies a new sort option, triggers GridTableEvent::Sort
      ///
      //virtual void setActiveSortConfig(int config_index, bool ascending = true) = 0;


      virtual ~GridTable()
      {}
   };


   /// @brief the smart-ptr-to-base that's used to work with the GridTable interface 
   ///
   using GridTablePtr = std::shared_ptr<GridTable>;


}  // namespace ctb::app