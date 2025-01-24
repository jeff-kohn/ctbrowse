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
      CellarTrackerGrid() = delete;
      CellarTrackerGrid(wxWindow* parent);

      void setGridTable(GridTableBase::GridTablePtr tbl_ptr);

   protected:
      void InitializeDefaults();
   };

}