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
#include <wx/dataview.h>
#include <wx/treectrl.h>

#include <map>
#include <optional>

namespace ctb::app
{
   /// @brief panel class that provides UI for sorting and filtering a grid
   class GridOptionsPanel final : public wxPanel, public IGridTableEventSink
   {
   public:

      /// @brief creates and initializes a panel for showing grid sort/filter options
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      [[nodiscard]] static GridOptionsPanel* create(wxWindow* parent, GridTableEventSourcePtr source);
   
      // no copy/move/assign, this class is created on the heap.
      GridOptionsPanel(const GridOptionsPanel&) = delete;
      GridOptionsPanel(GridOptionsPanel&&) = delete;
      GridOptionsPanel& operator=(const GridOptionsPanel&) = delete;
      GridOptionsPanel& operator=(GridOptionsPanel&&) = delete;
      ~GridOptionsPanel() override = default;
      
   private:
      using MaybeFilter = std::optional<GridTableFilter>;
      using FilterMap = std::map<wxTreeItemId, MaybeFilter>;
      using CheckCountMap = std::map<wxTreeItemId, int>;

      FilterMap             m_filters{};
      CheckCountMap         m_check_map{};
      ScopedEventSink       m_sink;           // no default init
      wxChoice*             m_sort_combo{};
      GridTableSortConfig   m_sort_config{};
      wxTreeCtrl*           m_filter_tree{};
      wxWithImages::Images  m_filter_tree_images{};

      // window creation
      void initControls();

      // implementation/helper functions
      void addFilter(wxTreeItemId item);
      void removeFilter(wxTreeItemId item);
      MaybeFilter getFilterForItem(wxTreeItemId item);
      wxArrayString getSortOptionList(GridTable* grid_table);
      bool isContainerNode(wxTreeItemId item);
      bool isMatchValueNode(wxTreeItemId item);
      bool isChecked(wxTreeItemId item);
      bool setChecked(wxTreeItemId item, bool checked = true);
      void updateFilterLabel(wxTreeItemId item);
      void toggleFilterSelection(wxTreeItemId item);

      /// event source related handlers
      void notify(GridTableEvent event, GridTable* grid_table) override;
      void onTableInitialize(GridTable* grid_table);
      void onTableSorted(GridTable* grid_table);
      void populateFilterTypes(GridTable* grid_table);

      /// event handlers
      void onSortOrderClicked(wxCommandEvent& event);
      void onSortSelection(wxCommandEvent& event);
      void onTreeFilterExpanding(wxTreeEvent& event);
      void onTreeFilterLeftClick(wxMouseEvent& event);

      /// @brief private ctor used by static create()
      explicit GridOptionsPanel(GridTableEventSourcePtr source);
   };

} // namespace ctb::app