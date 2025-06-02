/*******************************************************************
 * @file DatasetOptionsPanel.h
 *
 * @brief Header file for DatasetOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "CategorizedControls.h"

#include <ctb/model/ScopedEventSink.h>
#include <magic_enum/magic_enum.hpp>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/treectrl.h>

#include <map>
#include <optional>
#include <map>


// forward declare member pointers to avoid header pollution.
class wxBoxSizer;
class wxChoice;
class wxSpinCtrlDouble;
class wxStaticBoxSizer;
class wxTreeCtrl;
class wxStaticText;


namespace ctb::app
{

   class FilterCheckBox;


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
      [[nodiscard]] static auto create(wxWindow* parent, DatasetEventSourcePtr source) -> DatasetOptionsPanel*;
   
      // no copy/move/assign, this class is created on the heap.
      DatasetOptionsPanel(const DatasetOptionsPanel&) = delete;
      DatasetOptionsPanel(DatasetOptionsPanel&&) = delete;
      DatasetOptionsPanel& operator=(const DatasetOptionsPanel&) = delete;
      DatasetOptionsPanel& operator=(DatasetOptionsPanel&&) = delete;
      ~DatasetOptionsPanel() override = default;
      
   private:
      enum class ControlCategory : uint16_t
      {
         InStockFilter = 0,
         MinScoreFilter,
         ReadyToDrinkFilter,
      };
      using CategorizedControls = CategorizedControls<ControlCategory>;        // show/hide controls based on dataset context
      using MaybeMultiValFilter = std::optional<CtMultiMatchFilter>;           // associated with mm-filter node, will contain filter if node checked
      using TreeFilterMap       = std::map<wxTreeItemId, MaybeMultiValFilter>; // maps tree node to corresponding MaybeFilter 
      using CheckCountMap       = std::map<wxTreeItemId, int>;                 // used to track number of checked value nodes a given parent/filter node has 

      using FilterCheckboxes = std::map<ControlCategory, FilterCheckBox* >;

      bool                  m_sort_ascending     { true  }; // whether ascending sort order is active
      bool                  m_sort_descending    { false }; // whether descending sort ordes is active (yes we need both)
      CategorizedControls   m_categorized{};
      CheckCountMap         m_check_map{};                  // for keeping track of number of filter items selected 
      double                m_score_filter_val{};           // bound to min-score filter's spin-control value
      int                   m_sort_selection{ 0 };          // index of selected sort in combo, which matches a sort in availableSorts()
      FilterCheckboxes      m_filter_checkboxes{};          // checkbox controls for enabling/disabling different property filters
      IDataset::TableSort   m_sort_config{};                // the sort object that will be used to sort the dataset 
      ScopedEventSink       m_sink;               
      TreeFilterMap         m_multival_filters{}; 
      wxChoice*             m_sort_combo{};
      wxSpinCtrlDouble*     m_score_spin_ctrl{};
      wxStaticText*         m_dataset_title{};
      wxTreeCtrl*           m_filter_tree{};
      wxWithImages::Images  m_filter_tree_images{};

      // window creation
      void initControls();
      void createOptionFilters(wxStaticBoxSizer* parent);
      auto setTitle() -> bool;

      // multi-match filter impl
      void addMultiValFilter(wxTreeItemId item);
      auto getMultiValFilterForItem(wxTreeItemId item) -> MaybeMultiValFilter;
      auto isItemChecked(wxTreeItemId item) -> bool;
      auto isItemContainerNode(wxTreeItemId item) -> bool;
      auto isItemMatchValueNode(wxTreeItemId item) -> bool;
      void removeMultiValFilter(wxTreeItemId item);
      auto setMultiValChecked(wxTreeItemId item, bool checked = true) -> bool;
      void toggleFilterSelection(wxTreeItemId item);
      void updateFilterLabel(wxTreeItemId item);

      auto getSortOptionList(IDataset* dataset) -> wxArrayString;

      // Dataset-related evnet handlers
      void notify(DatasetEvent event) override;
      void onTableInitialize(IDataset* dataset);
      void onTableSorted(IDataset* dataset);
      void populateFilterTypes(IDataset* dataset);

      // event handlers
      void onInStockChecked(wxCommandEvent& event);
      void onMinScoreChanged(wxSpinDoubleEvent& event);
      void onMinScoreUpdateUI(wxUpdateUIEvent& event);
      void onMinScoreFilterChecked(wxCommandEvent& event);
      void onReadyToDrinkChecked(wxCommandEvent& event);
      void onSortOrderClicked(wxCommandEvent& event);
      void onSortSelection(wxCommandEvent& event);
      void onTreeFilterExpanding(wxTreeEvent& event);
      void onTreeFilterLeftClick(wxMouseEvent& event);

      /// @brief private ctor used by static create()
      explicit DatasetOptionsPanel(DatasetEventSourcePtr source);
   };

} // namespace ctb::app