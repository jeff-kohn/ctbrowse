/*******************************************************************
 * @file DatasetOptionsPanel.h
 *
 * @brief Header file for DatasetOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/ScopedEventSink.h"

#include <wx/choice.h>
#include <wx/dataview.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
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
      using MaybeFilter = std::optional<CtStringFilter>;
      using FilterMap = std::map<wxTreeItemId, MaybeFilter>; // map tree node to corresponding filter config
      using CheckCountMap = std::map<wxTreeItemId, int>;     // for keeping track of number of filter items selected 

      CheckCountMap         m_check_map{};
      bool                  m_enable_score_filter{ false };
      wxStaticBoxSizer*     m_filter_options_box{};
      bool                  m_instock_only{ constants::CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT };
      wxTreeCtrl*           m_filter_tree{};
      wxWithImages::Images  m_filter_tree_images{};
      FilterMap             m_filters{};
      wxSpinCtrlDouble*     m_score_spin_ctrl{};
      double                m_score_filter_val{};
      ScopedEventSink       m_sink;           // no default init
      wxChoice*             m_sort_combo{};
      CtSortConfig          m_sort_config{};

      // window creation
      void initControls();

      // implementation functions for the property filter treeview
      void addPropFilter(wxTreeItemId item);
      void removePropFilter(wxTreeItemId item);
      MaybeFilter getPropFilterForItem(wxTreeItemId item);
      wxArrayString getSortOptionList(IDataset* dataset);
      bool isChecked(wxTreeItemId item);
      bool isContainerNode(wxTreeItemId item);
      bool isMatchValueNode(wxTreeItemId item);
      bool setMatchValueChecked(wxTreeItemId item, bool checked = true);
      void toggleFilterSelection(wxTreeItemId item);
      void updateFilterLabel(wxTreeItemId item);

      // in-stock and score filters.
      void enableInStockFilter(bool enable = true);
      void resetInStockCheckbox();

      /// event source related handlers
      void notify(DatasetEvent event) override;
      void onTableInitialize(IDataset* dataset);
      void onTableSorted(IDataset* dataset);
      void populateFilterTypes(IDataset* dataset);

      /// event handlers
      void OnInStockChecked(wxCommandEvent& event);
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