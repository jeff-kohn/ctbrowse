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
#include "model/CtDatasetOptions.h"

#include <ctb/model/ScopedEventSink.h>

#include <wx/panel.h>
#include <wx/spinctrl.h>


// forward declare member pointers to avoid header pollution.
class wxBoxSizer;
class wxChoice;
class wxStaticBoxSizer;
class wxStaticText;
class wxSlider;


namespace ctb::app
{

   class FilterCheckBox;
   class SpinDoubleFilterCtrl;
   class MultiValueFilterTree;


   /// @brief panel class that provides UI for setting sorting and filtering options
   ///
   class DatasetOptionsPanel final : public wxPanel, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing sort/filter options
      ///
      /// throws a ctb::Error if source == nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window
      [[nodiscard]] static auto create(wxWindow& parent, DatasetEventSourcePtr source) noexcept(false) -> DatasetOptionsPanel* ;   


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
         MinPriceFilter,
         MaxPriceFilter,
      };
      using CategorizedControls = CategorizedControls<ControlCategory>;         // show/hide controls based on dataset context
      using FilterCheckboxes    = std::map<ControlCategory, FilterCheckBox* >;  // checkbox controls for each of the check filters (see ControlCategory enum)

      bool                  m_sort_ascending     { true  }; // whether ascending sort order is active
      bool                  m_sort_descending    { false }; // whether descending sort ordes is active (yes we need both)
      CategorizedControls   m_categorized{};
      int                   m_sort_selection{ 0 };          // index of selected sort in combo, which matches a sort in availableSorts()
      int                   m_min_price{};
      int                   m_max_price{};
      FilterCheckboxes      m_filter_checkboxes{};          // checkbox controls for enabling/disabling different property filters
      IDataset::TableSort   m_sort_config{};                // the sort object that will be used to sort the dataset 
      MultiValueFilterTree* m_filter_tree{};
      ScopedEventSink       m_sink;               
      wxChoice*             m_sort_combo{};
      SpinDoubleFilterCtrl* m_min_score_filter_ctrl{};
      SpinDoubleFilterCtrl* m_min_price_filter_ctrl{};
      SpinDoubleFilterCtrl* m_max_price_filter_ctrl{};
      wxStaticText*         m_dataset_title{};

      // window creation
      void initControls();
      void createOptionFilters(wxStaticBoxSizer* parent);
      auto setTitle() -> bool;

      auto getSortOptionList(IDataset* dataset) -> wxArrayString;

      // Dataset-related event handlers
      void notify(DatasetEvent event) override;
      void onDatasetInitialize(IDataset* dataset);
      void onTableSorted(IDataset* dataset);
      auto getDataset() const noexcept(false) -> DatasetPtr ; // throws rather than return nullptr

      // event handlers
      void onFilterChecked(ControlCategory cat);
      void onFilterInStockChecked(wxCommandEvent& event);
      void onFilterReadyToDrinkChecked(wxCommandEvent& event);
      void onSortOrderClicked(wxCommandEvent& event);
      void onSortSelection(wxCommandEvent& event);

      /// @brief private ctor used by static create()
      explicit DatasetOptionsPanel(DatasetEventSourcePtr source);
   };

} // namespace ctb::app