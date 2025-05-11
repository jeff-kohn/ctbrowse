/*******************************************************************
 * @file DatasetOptionsPanel.h
 *
 * @brief Header file for DatasetOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include <ctb/model/ScopedEventSink.h>
#include <ctb/tables/detail/SortConfig.h>

#include <wx/choice.h>
#include <wx/dataview.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/treectrl.h>

#include <map>
#include <optional>

namespace ctb::app
{
   /// @brief panel class that provides UI for setting sorting and filtering options
   ///
   class DatasetOptionsPanel final : public wxPanel, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing sort/filter options
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      [[nodiscard]] static DatasetOptionsPanel* create(wxWindow* parent, DatasetEventSourcePtr source);
   
      // no copy/move/assign, this class is created on the heap.
      DatasetOptionsPanel(const DatasetOptionsPanel&) = delete;
      DatasetOptionsPanel(DatasetOptionsPanel&&) = delete;
      DatasetOptionsPanel& operator=(const DatasetOptionsPanel&) = delete;
      DatasetOptionsPanel& operator=(DatasetOptionsPanel&&) = delete;
      ~DatasetOptionsPanel() override = default;
      
   private:
      using MaybeFilter   = std::optional<CtMultiMatchFilter>;
      using FilterMap     = std::map<wxTreeItemId, MaybeFilter>; // map tree node to corresponding MaybeFilter 
      using CheckCountMap = std::map<wxTreeItemId, int>;     

      bool                  m_enable_score_filter{ false }; // whether score filter is active
      bool                  m_instock_only{ constants::CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT };
      bool                  m_sort_ascending{ true };       // whether ascending sort order is active
      bool                  m_sort_descending{ false };     // whether descending sort ordes is active
      CheckCountMap         m_check_map{};                  // for keeping track of number of filter items selected 
      double                m_score_filter_val{};
      int                   m_sort_selection{0};            // index of selected sort in combo, which matches a sort in availableSorts()
      IDataset::TableSort   m_sort_config{};                // the sort object that will be used to sort the dataset 
      FilterMap             m_filters{};
      ScopedEventSink       m_sink;           
      wxChoice*             m_sort_combo{};
      wxSpinCtrlDouble*     m_score_spin_ctrl{};
      wxStaticBoxSizer*     m_filter_options_box{};         
      wxTreeCtrl*           m_filter_tree{};
      wxWithImages::Images  m_filter_tree_images{};

      // window creation
      void initControls();

      // implementation functions for the property filter tree-view
      void addPropFilter(wxTreeItemId item);
      void removePropFilter(wxTreeItemId item);
      auto getPropFilterForItem(wxTreeItemId item) -> MaybeFilter;
      auto getSortOptionList(IDataset* dataset) -> wxArrayString;
      auto isChecked(wxTreeItemId item) -> bool;
      auto isContainerNode(wxTreeItemId item) -> bool;
      auto isMatchValueNode(wxTreeItemId item) -> bool;
      auto setMatchValueChecked(wxTreeItemId item, bool checked = true) -> bool;
      void toggleFilterSelection(wxTreeItemId item);
      void updateFilterLabel(wxTreeItemId item);

      // in-stock filters.
      void enableInStockFilter(bool enable = true);
      void resetInStockCheckbox();

      /// event source related handlers
      void notify(DatasetEvent event) override;
      void onTableInitialize(IDataset* dataset);
      void onTableSorted(IDataset* dataset);
      void populateFilterTypes(IDataset* dataset);

      /// event handlers
      void onInStockChecked(wxCommandEvent& event);
      void onMinScoreChanged(wxSpinDoubleEvent& event);
      void onMinScoreFilterChecked(wxCommandEvent& event);
      void onSortOrderClicked(wxCommandEvent& event);
      void onSortSelection(wxCommandEvent& event);
      void onTreeFilterExpanding(wxTreeEvent& event);
      void onTreeFilterLeftClick(wxMouseEvent& event);

      /// @brief private ctor used by static create()
      explicit DatasetOptionsPanel(DatasetEventSourcePtr source);
   };

} // namespace ctb::app