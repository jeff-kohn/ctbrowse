/*******************************************************************
 * @file GridOptionsPanel.h
 *
 * @brief Header file for GridOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "grid/ScopedEventSink.h"

#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>


namespace ctb::app
{
   /// @brief panel class that provides UI for sorting and filtering a grid
   class GridOptionsPanel final : public wxPanel, public IGridTableEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing grid sort/filter options
      ///
      /// throws a ctb::Error parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      static [[nodiscard]] GridOptionsPanel* create(wxWindow* parent, GridTableEventSourcePtr source);
   
   private:
      ScopedEventSink         m_sink;
      wxChoice*               m_sort_combo{};
      int                     m_sort_idx{};

      GridOptionsPanel(GridTableEventSourcePtr source);
      void initControls();
      void notify(GridTableEvent event, IGridTable* grid_table) override;
      void populateSortOptions(IGridTable* grid_table);
      void updateSortSelection(IGridTable* grid_table);

      void onSortSelection(wxCommandEvent& event);
      void onSortDirection(wxCommandEvent& event);

      // no copy/move/assign, this class is created on the heap.
      GridOptionsPanel(const GridOptionsPanel&) = delete;
      GridOptionsPanel(GridOptionsPanel&&) = delete;
      GridOptionsPanel& operator=(const GridOptionsPanel&) = delete;
      GridOptionsPanel& operator=(GridOptionsPanel&&) = delete;
   };

} // namespace ctb::app