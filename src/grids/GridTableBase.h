#/*******************************************************************
 * @file GridTableBase.h
 *
 * @brief Header file for the GridTableBase class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include <wx/grid.h>
#include <memory>

namespace ctb
{
   /// @brief base class used by all of our GridTable objects.
   class GridTableBase : public wxGridStringTable
   {
   public:
      /// @brief the smart-ptr-to-base that this class returns to callers.
      using GridTablePtr = std::shared_ptr<GridTableBase>;

      /// @brief Sets up the formatting options in the calling grid to match our data fields
      virtual void configureGridColumns(wxGridCellAttrPtr default_attr) = 0;

      /// @brief filter does substring matching on ANY column in the table view
      virtual bool filterBySubstring(std::string_view substr) = 0;

      /// @brief filter that does substring matching on the specified column
      virtual bool filterBySubstring(std::string_view substr, size_t col_idx) = 0;

      /// @brief clear the substring filter.
      virtual void clearSubStringFilter() = 0;

      virtual size_t getTotalRowCount() const = 0;
      virtual size_t getFilteredRowCount() const  = 0;
   };



}