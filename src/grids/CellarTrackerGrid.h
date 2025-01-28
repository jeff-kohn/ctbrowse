/*******************************************************************
 * @file CTGrid.h
 *
 * @brief Header file for the class CTGrid
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "grids/GridTableBase.h"

#include <wx/grid.h>


namespace ctb
{
   /// @brief grid class used for displaying CellarTracker table data
   ///
   /// 
   class CellarTrackerGrid : public wxGrid
   {
   public:
      CellarTrackerGrid(wxWindow* parent);
      ~CellarTrackerGrid() override = default;


      /// @brief type alias used for our table's ptr-to-base 
      using GridTablePtr = GridTableBase::GridTablePtr;

      /// @brief initialize (or re-initialize) the grid with the provided table
      ///         
      /// an exception will be thrown if you pass in a nullptr
      void setGridTable(GridTablePtr tbl);


      /// @brief filter the table by performing a substring search across all columns
      ///
      /// note that class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      void filterBySubstring(std::string_view substr);


      /// @brief filter the table by performing a substring search on the specified column
      ///
      /// note that class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      void filterBySubstring(std::string_view substr, size_t col_idx);

      /// @brief clear/reset the substring filter
      void clearSubStringFilter();

      CellarTrackerGrid() = delete;
      CellarTrackerGrid(const CellarTrackerGrid&) = delete;
      CellarTrackerGrid(CellarTrackerGrid&&) = delete;

   protected:
      GridTablePtr m_table{};
      void InitializeDefaults();
   };

}