/*******************************************************************
 * @file CellarTrackerGrid.cpp
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "grid/CellarTrackerGrid.h"


namespace ctb::app
{

   void CellarTrackerGrid::initGrid()
   {
      EnableEditing(false);
      EnableDragGridSize(false);
      UseNativeColHeader(true);
   }


   [[nodiscard]] CellarTrackerGrid* CellarTrackerGrid::create(wxWindow* parent, GridTableEventSourcePtr source)
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!parent)
      {
         assert("parent parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<CellarTrackerGrid> wnd{ new CellarTrackerGrid{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initGrid();
      return wnd.release();
   }


   CellarTrackerGrid::~CellarTrackerGrid()
   {
      // wxGrid accesses the table pointer from its destructor if not null, 
      // and our table may already be destroyed by then.
      SetTable(nullptr);
   }


   void CellarTrackerGrid::setGridTable(GridTablePtr tbl)
   {
      // we save our own copy of the ptr, because we need access to 
      // GridTable methods, and GetTable() returns wxGridTableBase*.
      // We store a shared_ptr instead of the raw ptr to prevent the
      // table from being deleted out from under us (object lifetimes
      // can get tricky with wxWindow-derived classes).
      m_grid_table = tbl;
      {
         wxGridUpdateLocker lock(this);
         
         // assign the table and some other initial settings.
         SetTable(m_grid_table.get(), false);
         HideRowLabels();
         SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
         SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);
         //SetSortingColumn(0, true);

         // set the font size for the grid
         auto attr = GetOrCreateCellAttrPtr(0, 0);
         assert(attr);
         wxFont font{ attr->GetFont() };
         font.SetPointSize(10);
         attr->SetFont(font);

         // give the grid table a chance to configure column formatting
         m_grid_table->configureGridColumns(attr);

         AutoSizeColumns(false);
         AutoSizeRows(true);
      }
      ForceRefresh();
   }


   bool CellarTrackerGrid::filterBySubstring(std::string_view substr)
   {
      if (!m_grid_table)
         throw Error{ constants::ERROR_STR_NO_GRID_TABLE, Error::Category::UiError };


      if (substr.empty())
      {
         clearSubStringFilter();
         return false;
      }

      {
         wxGridUpdateLocker lock(this);
         if (m_grid_table->filterBySubstring(substr))
         {
            // calling SetTable with the same ptr is fine, it forces grid to re-fetch the data.
            setGridTable(m_grid_table);
            return true;
         }
         else {
            wxGetApp().displayInfoMessage(constants::INFO_MSG_NO_MATCHING_ROWS);
            return false;
         }
      }
   }


   bool CellarTrackerGrid::filterBySubstring(std::string_view substr, int col_idx)
   {
      if (!m_grid_table)
         throw Error{ constants::ERROR_STR_NO_GRID_TABLE, Error::Category::UiError };
      
      wxBusyCursor busy{};
      {
         wxGridUpdateLocker lock(this);
         if (m_grid_table->filterBySubstring(substr, col_idx))
         {
            // calling setTable with the same ptr is fine, it forces grid to re-fetch the data.
            setGridTable(m_grid_table);
            return true;
         }
         else {
            wxGetApp().displayInfoMessage(constants::INFO_MSG_NO_MATCHING_ROWS);
            return false;
         }
      }
   }


   void CellarTrackerGrid::clearSubStringFilter()
   {
      if (!m_grid_table)
         throw Error{ constants::ERROR_STR_NO_GRID_TABLE, Error::Category::UiError };

      wxBusyCursor busy{};
      {
         wxGridUpdateLocker lock(this);
         m_grid_table->clearSubStringFilter();
         setGridTable(m_grid_table);
      }
   }


   void CellarTrackerGrid::notify(GridTableEvent event, [[maybe_unused]] GridTable* grid_table)
   {
      switch (event)
      { 
         case GridTableEvent::TableRemove:
            SetTable(nullptr);
            m_grid_table.reset();
            break;

         case GridTableEvent::TableInitialize:
         case GridTableEvent::Sort:
         case GridTableEvent::Filter:
         case GridTableEvent::SubStringFilter:
            setGridTable(m_sink.getTable());   // we need the ref-counted smart-ptr
            break;

         case GridTableEvent::RowSelected:
            break;
         default:
            break;
      }
   }


} // namespace ctb::app