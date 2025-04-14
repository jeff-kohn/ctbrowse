/*******************************************************************
 * @file CellarTrackerGrid.h
 *
 * @brief Header file for the class CellarTrackerGrid
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/ScopedEventSink.h"

#include <wx/grid.h>


namespace ctb::app
{
   /// @brief grid class used for displaying CellarTracker table data
   ///
   /// 
   class CellarTrackerGrid : public wxGrid, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a grid window for displaying CellarTracker data
      ///
      /// throws a ctb::Error parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      [[nodiscard]] static CellarTrackerGrid* create(wxWindow* parent, DatasetEventSourcePtr source);

      /// @brief filter the table by performing a substring search across all columns
      ///
      /// note this class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      /// 
      bool filterBySubstring(std::string_view substr);

      /// @brief filter the table by performing a substring search on the specified column
      ///
      /// note this class only supports a single substring filter, subsequent calls to
      /// either overload will overwrite any previous substring filter.
      /// 
      bool filterBySubstring(std::string_view substr, int col_idx);

      /// @brief clear/reset the substring filter
      ///
      void clearSubStringFilter();

      /// @brief destructor
      ///
      ~CellarTrackerGrid() override;

      // no copy/move/assign, this class is created on the heap.
      CellarTrackerGrid(const CellarTrackerGrid&) = delete;
      CellarTrackerGrid(CellarTrackerGrid&&) = delete;
      CellarTrackerGrid& operator=(const CellarTrackerGrid&) = delete;
      CellarTrackerGrid& operator=(CellarTrackerGrid&&) = delete;

   protected:
      IDatasetPtr    m_grid_table{};
      ScopedEventSink m_sink;
      
      void notify(DatasetEvent event) override;
      void onGridCellChanging(wxGridEvent& event);
      void onDestroyWindow(wxWindowDestroyEvent& event);


      void initGrid();
      void setGridTable(IDatasetPtr tbl);

      /// @brief private ctor used by static create()
      explicit CellarTrackerGrid(DatasetEventSourcePtr source) : m_sink{ this, source }
      {}
   };

}  // namespace ctb::app