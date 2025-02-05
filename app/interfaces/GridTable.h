#/*******************************************************************
 * @file GridTable.h
 *
 * @brief Header file for the IGridTable interface
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include <wx/grid.h>
#include <set>
#include <memory>


namespace ctb::app
{
   struct IGridTableEventSource;
   using GridTableEventSourcePtr = std::shared_ptr<IGridTableEventSource>;


   /// @brief our data "model" interface, provides an interface for accessing the data for our views
   /// 
   /// users of this interface should gain access through 
   /// IGridTableEventSource::getTable() or using the ptr supplied when
   /// handling IGridTableEventSource::notify()
   /// 
   class IGridTable : public wxGridStringTable
   {
   public:
      virtual ~IGridTable()
      {}


      /// @brief Sets up the formatting options in the calling grid to match this grid table's fields
      virtual void configureGridColumns(wxGridCellAttrPtr default_attr) = 0;


      /// @brief filter does substring matching on ANY column in the table view
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers GridTableEvent::SubStringFilter
      virtual bool filterBySubstring(std::string_view substr) = 0;


      /// @brief sets filter that does substring matching on the specified column
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers GridTableEvent::SubStringFilter
      virtual bool filterBySubstring(std::string_view substr, size_t col_idx) = 0;


      /// @brief clear the substring filter
      ///
      /// triggers GridTableEvent::SubStringFilter
      virtual void clearSubStringFilter() = 0;


      /// @brief returns the total number of records in the underlying dataset
      virtual size_t totalRowCount() const = 0;


      /// @brief returns the number of records with filters applied.
      virtual size_t filteredRowCount() const  = 0;


      /// @brief contains a name and index of a sort option
      struct SortOptionName
      {
         size_t            sort_index{};
         std::string_view  sort_name{};
      };


      /// @brief retrieves list of available sortOptions, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      virtual std::vector<SortOptionName> availableSortOptions() const = 0;


      /// @brief returns the currently active sort option
      virtual SortOptionName currentSortSelection() const = 0;


      /// @brief specifies a new sort option, triggers GridTableEvent::Sort
      virtual void setSortSelection(size_t sort_index) = 0;
   };

   /// @brief the smart-ptr-to-base that this class returns to callers.
   using GridTablePtr = std::shared_ptr<IGridTable>;



}  // namespace ctb::app