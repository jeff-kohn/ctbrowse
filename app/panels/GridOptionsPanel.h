/*******************************************************************
 * @file GridOptionsPanel.h
 *
 * @brief Header file for GridOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "grid/GridTableFilterMgr.h"
#include "grid/ScopedEventSink.h"

#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/dataview.h>
#include <wx/treelist.h>


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
      GridTable::SortConfig   m_sort_config{};
      wxTreeListCtrl*         m_filter_tree{};

      /// @brief called when there are updates to the table 
      void notify(GridTableEvent event, GridTable* grid_table) override;

      void initControls();
      wxArrayString getSortOptionList(GridTable* grid_table);
      void populateFilterTypes(GridTable* grid_table);
      void populateChoicesForFilter(GridTable* grid_table);

      // event handlers
      void onSortSelection(wxCommandEvent& event);
      void onSortOrderClicked(wxCommandEvent& event);
      void onTableInitialize(GridTable* grid_table);
      void onTableSorted(GridTable* grid_table);
      void OnFilterTreeItemExpand(wxDataViewEvent& event);

      /// @brief private ctor used by static create()
      GridOptionsPanel(GridTableEventSourcePtr source);

      // no copy/move/assign, this class is created on the heap.
      GridOptionsPanel(const GridOptionsPanel&) = delete;
      GridOptionsPanel(GridOptionsPanel&&) = delete;
      GridOptionsPanel& operator=(const GridOptionsPanel&) = delete;
      GridOptionsPanel& operator=(GridOptionsPanel&&) = delete;
   };

} // namespace ctb::app