#include "grids/CellarTrackerGrid.h"

namespace ctb
{


   CellarTrackerGrid::CellarTrackerGrid(wxWindow* parent) : wxGrid(parent, wxID_ANY)
   {
      InitializeDefaults();
   }


   void CellarTrackerGrid::setGridTable(GridTableMgr::GridTablePtr tbl_ptr)
   {
      SetTable(tbl_ptr.get(), false);
      //tbl_ptr->ConfigureColumns(m_grid);
      SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);
      SetSortingColumn(0, true);
      AutoSizeColumns(false);
      Refresh();
   }


   void CellarTrackerGrid::InitializeDefaults()
   {
      EnableEditing(false);
      EnableDragGridSize(false);
      SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
      SetLabelBackgroundColour(wxColour("#FFFFFF"));
      SetMargins(0, 0);
      HideRowLabels();
      UseNativeColHeader(true);
   }


}