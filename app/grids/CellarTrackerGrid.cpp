#include "grids/CellarTrackerGrid.h"

#include "App.h"

#include "ctb/ctb.h"

namespace ctb::app
{

   CellarTrackerGrid::CellarTrackerGrid(wxWindow* parent) : 
      wxGrid(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxBORDER_THEME)
   {
      InitializeDefaults();
   }


   void CellarTrackerGrid::setGridTable(GridTableBase::GridTablePtr tbl)
   {
      assert(tbl);
      if (!tbl)
         throw Error{ constants::ERROR_NULL_POINTER, Error::Category::ArgumentError };

      // we save our own copy of the ptr, because we need access to 
      // GridTableBase methods, and GetTable() returns wxGridTableBase*
      m_table = tbl;
      {
         wxGridUpdateLocker lock(this);
         
         // assign the table and some other initial settings.
         SetTable(m_table.get(), false);
         SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);
         SetSortingColumn(0, true);

         // set the font size for the grid
         auto attr = GetOrCreateCellAttrPtr(0, 0);
         assert(attr);
         wxFont font{ attr->GetFont() };
         font.SetPointSize(10);
         attr->SetFont(font);

         // give the grid table a chance to configure column formatting
         m_table->configureGridColumns(attr);

         AutoSizeColumns(false);
         AutoSizeRows(true);
      }
      ForceRefresh();
   }

   void CellarTrackerGrid::filterBySubstring(std::string_view substr)
   {
      if (!m_table)
         throw Error{ constants::ERROR_NO_GRID_TABLE, Error::Category::UiError };


      if (substr.empty())
      {
         clearSubStringFilter();
         return;
      }

      {
         wxGridUpdateLocker lock(this);
         if (m_table->filterBySubstring(substr))
         {
            // calling SetTable with the same ptr is fine, it forces grid to re-fetch the data.
            setGridTable(m_table);
         }
         else {
            wxGetApp().displayInfoMessage(constants::INFO_MSG_NO_MATCHING_ROWS);
         }
      }
   }

   void CellarTrackerGrid::filterBySubstring(std::string_view substr, size_t col_idx)
   {
      if (!m_table)
         throw Error{ constants::ERROR_NO_GRID_TABLE, Error::Category::UiError };
      
      wxBusyCursor busy{};
      {
         wxGridUpdateLocker lock(this);
         if (m_table->filterBySubstring(substr, col_idx))
         {
            // calling setTable with the same ptr is fine, it forces grid to re-fetch the data.
            setGridTable(m_table);
         }
         else {
            wxGetApp().displayInfoMessage(constants::INFO_MSG_NO_MATCHING_ROWS);
         }
      }
   }

   void CellarTrackerGrid::clearSubStringFilter()
   {
      if (!m_table)
         throw Error{ constants::ERROR_NO_GRID_TABLE, Error::Category::UiError };

      wxBusyCursor busy{};
      {
         wxGridUpdateLocker lock(this);
         m_table->clearSubStringFilter();
         setGridTable(m_table);
      }
   }


   void CellarTrackerGrid::InitializeDefaults()
   {
      EnableEditing(false);
      EnableDragGridSize(false);
      SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
      HideRowLabels();
      UseNativeColHeader(true);
   }


} // namespace ctb::app