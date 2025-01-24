#include "grids/CellarTrackerGrid.h"


namespace ctb
{

   CellarTrackerGrid::CellarTrackerGrid(wxWindow* parent) : wxGrid(parent, wxID_ANY)
   {
      InitializeDefaults();
   }


   void CellarTrackerGrid::setGridTable(GridTableBase::GridTablePtr tbl_ptr)
   {
      {
         wxGridUpdateLocker lock(this);

         // assign the table and some other initial settings.
         SetTable(tbl_ptr.get(), false);
         SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);
         SetSortingColumn(0, true);

         // set the font size for the grid
         auto attr_ptr = GetOrCreateCellAttrPtr(0, 0);
         assert(attr_ptr);
         wxFont font{ attr_ptr->GetFont() };
         font.SetPointSize(10);
         attr_ptr->SetFont(font);

         // give the grid table a chance to configure column formatting
         tbl_ptr->configureGridColumns(attr_ptr);

         AutoSizeColumns(false);
         AutoSizeRows(true);
      }
      Refresh();
   }


   void CellarTrackerGrid::InitializeDefaults()
   {
      EnableEditing(false);
      EnableDragGridSize(false);
      SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
      HideRowLabels();
      UseNativeColHeader(true);
   }


}