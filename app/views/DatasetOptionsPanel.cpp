/*******************************************************************
 * @file DatasetOptionsPanel.cpp
 *
 * @brief implementation file for the DatasetOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "views/DatasetOptionsPanel.h"
#include "views/FilterCheckBox.h" 
#include "views/MultiValueFilterTree.h"
#include "wx_helpers.h"

#include "model/CtDatasetOptions.h"

#include <ctb/utility_chrono.h>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/dataview.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/valgen.h>
#include <wx/wupdlock.h>

#include <string>
#include <string_view>

namespace ctb::app
{
   namespace 
   {
      constexpr int IMG_CONTAINER = 0;
      constexpr int IMG_UNCHECKED = 1;
      constexpr int IMG_CHECKED = 2;

      void forceLayoutUpdate(wxWindow* window)
      {
         window->GetSizer()->Layout();
         window->SendSizeEvent();
         window->Update();
      }
      
   } // namespace
   

   [[nodiscard]] auto DatasetOptionsPanel::create(wxWindow& parent, DatasetEventSourcePtr source) noexcept(false) -> DatasetOptionsPanel*
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<DatasetOptionsPanel> wnd{ new DatasetOptionsPanel{source} };
      if (!wnd->Create(&parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   DatasetOptionsPanel::DatasetOptionsPanel(DatasetEventSourcePtr source) : m_sink{ this, source }
   {}


   void DatasetOptionsPanel::initControls()
   {
      using namespace ctb::constants;

      const auto default_border = wxSizerFlags::GetDefaultBorder();

      // panel shouldn't grow infinitely
      SetMaxSize(ConvertDialogToPixels(wxSize{ 150, WX_UNSPECIFIED_VALUE }));
      SetMinSize(ConvertDialogToPixels(wxSize{ 100, WX_UNSPECIFIED_VALUE }));

      // defines the rows of controls in our panel
      auto* top_sizer = new wxBoxSizer{ wxVERTICAL };
      top_sizer->AddSpacer(default_border);

      // Dataset title
      auto title_font{ GetFont().MakeLarger().MakeBold()};
      const auto heading_color = wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT);
      const auto title_border_size = FromDIP(10);
      m_dataset_title = new wxStaticText{ this, wxID_ANY, "" };
      m_dataset_title->SetFont(title_font);
      m_dataset_title->SetForegroundColour(heading_color);
      top_sizer->Add(m_dataset_title, wxSizerFlags{}.Expand().Border(wxALL, title_border_size));

      // sort options box
      auto* sort_options_box = new wxStaticBoxSizer(wxVERTICAL, this, LBL_SORT_OPTIONS);

      // sort fields combo
      m_sort_combo = new wxChoice(sort_options_box->GetStaticBox(), wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_selection));
      sort_options_box->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxALL));

      // ascending sort order radio. 
      auto opt_ascending = new wxRadioButton{
         sort_options_box->GetStaticBox(),
         wxID_ANY, 
         LBL_SORT_ASCENDING,
         wxDefaultPosition, 
         wxDefaultSize, 
         wxRB_GROUP
      };
      opt_ascending->SetValue(true);
      opt_ascending->SetValidator(wxGenericValidator{ &m_sort_ascending });
      sort_options_box->Add(opt_ascending, wxSizerFlags{}.Expand().Border(wxALL));

      // descending sort order radio. Since the radio buttons aren't in a group box, the validator treats them as individual bools
      // so we have a separate flag for the descending radio that we have to manually keep in sync (see onTableSorted)
      auto opt_descending = new wxRadioButton{ sort_options_box->GetStaticBox(), wxID_ANY, LBL_SORT_DESCENDING };
      opt_descending->SetValidator(wxGenericValidator{ &m_sort_descending });
      sort_options_box->Add(opt_descending, wxSizerFlags{1}.Expand().Border(wxALL));
      top_sizer->Add(sort_options_box, wxSizerFlags().Expand().Border(wxALL));
      top_sizer->AddSpacer(default_border);

      // filter options box, contains filter tree and checkboxes
      auto* filter_options_box = new wxStaticBoxSizer(wxVERTICAL, this, LBL_FILTER_OPTIONS);

      // filter tree control
      m_filter_tree = MultiValueFilterTree::create(*filter_options_box->GetStaticBox(), m_sink.getSource());
      m_filter_tree->SetMaxSize(ConvertDialogToPixels(wxSize(-1, 500)));
      m_filter_tree->SetMinSize(ConvertDialogToPixels(wxSize(-1, 100)));
      filter_options_box->Add(m_filter_tree, wxSizerFlags(2).Expand().Border(wxALL));
      filter_options_box->AddSpacer(default_border);

      // checkbox filters
      createOptionFilters(filter_options_box);
      top_sizer->Add(filter_options_box, wxSizerFlags(1).Expand().Border(wxALL));

      // Save as Default button, will go side by side with...
      auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
      auto* btn_save_default = new wxButton{ this, wxID_ANY, LBL_BTN_SAVE_DEFAULT };
      btn_save_default->SetToolTip(LBL_BTN_SAVE_DEFAULT_TIP);
      button_sizer->Add(btn_save_default, wxSizerFlags(1).Expand().Border(wxALL));

      // ...Clear Filters button
      auto* btn_clear_filters = new wxButton(this, wxID_ANY, LBL_BTN_CLEAR_FILTERS);
      btn_clear_filters->SetToolTip(LBL_BTN_CLEAR_FILTERS_TIP);
      button_sizer->Add(btn_clear_filters, wxSizerFlags(1).Expand().Border(wxALL));
      top_sizer->Add(button_sizer, wxSizerFlags().Expand().Border(wxALL));

      // finalize layout
      top_sizer->AddStretchSpacer(2);
      SetSizer(top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &DatasetOptionsPanel::onSortSelection, this);
      m_score_spin_ctrl->Bind(wxEVT_SPINCTRLDOUBLE, &DatasetOptionsPanel::onMinScoreChanged,  this);
      m_score_spin_ctrl->Bind(wxEVT_UPDATE_UI,      &DatasetOptionsPanel::onMinScoreUpdateUI, this);    
      opt_ascending->Bind( wxEVT_RADIOBUTTON, &DatasetOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &DatasetOptionsPanel::onSortOrderClicked, this);
      m_filter_checkboxes[ControlCategory::InStockFilter     ]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterInStockChecked,      this);
      m_filter_checkboxes[ControlCategory::MinScoreFilter    ]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterMinScoreChecked,     this);     
      m_filter_checkboxes[ControlCategory::ReadyToDrinkFilter]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterReadyToDrinkChecked, this);     
      btn_save_default->Bind(wxEVT_BUTTON, &DatasetOptionsPanel::onSaveDefault, this);
      btn_clear_filters->Bind(wxEVT_BUTTON, &DatasetOptionsPanel::onClearFilters, this);
   }


   void DatasetOptionsPanel::createOptionFilters(wxStaticBoxSizer* parent)
   {
      using enum CtProp;
      using enum ControlCategory;
      using namespace ctb::constants;

      // in-stock filter
      m_filter_checkboxes[InStockFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_IN_STOCK_ONLY, { QtyOnHand }, uint16_t{0}, CtPropFilterPredicate{ CtPredicateType::Greater } }};
      parent->Add(m_filter_checkboxes[InStockFilter], wxSizerFlags().Border(wxALL));

      // embedded sizer to place score spin-ctrl next to checkbox
      auto* min_score_sizer = new wxBoxSizer(wxHORIZONTAL);

      // min-score filter checkbox
      m_filter_checkboxes[MinScoreFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_MIN_SCORE, { CtScore, MyScore }, FILTER_SCORE_DEFAULT, CtPropFilterPredicate{ CtPredicateType::GreaterEqual } }};
      min_score_sizer->Add(m_filter_checkboxes[MinScoreFilter], wxSizerFlags{}.Center().Border(wxLEFT|wxTOP|wxBOTTOM));

      //  min-score filter spin-ctrl
      m_score_spin_ctrl = new wxSpinCtrlDouble
      {
         parent->GetStaticBox(),
         wxID_ANY,
         wxEmptyString, wxDefaultPosition, wxDefaultSize, 
         wxSP_ARROW_KEYS | wxALIGN_RIGHT, 
         FILTER_SCORE_MIN,     FILTER_SCORE_MAX, 
         FILTER_SCORE_DEFAULT, FILTER_SCORE_INCR
      };
      m_score_spin_ctrl->SetDigits(FILTER_SCORE_DIGITS);
      m_score_spin_ctrl->Enable(false);
      min_score_sizer->Add(m_score_spin_ctrl, wxSizerFlags{}.Border(wxRIGHT|wxTOP|wxBOTTOM));

      // add embedded sizer to parent
      parent->Add(min_score_sizer, wxSizerFlags{});

      // ready-to-drink filter, matches if any formula calculates RTD >= 0;
      auto props = { RtdQtyDefault, RtdQtyLinear, RtdQtyBellCurve, RtdQtyEarlyCurve, RtdQtyLateCurve, RtdQtyFastMaturing, RtdQtyEarlyAndLate, RtdQtyBottlesPerYear, };
      m_filter_checkboxes[ReadyToDrinkFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_READY_TO_DRINK, props, FILTER_AVAILABLE_MIN_QTY, CtPropFilterPredicate{ CtPredicateType::GreaterEqual } }};
      parent->Add(m_filter_checkboxes[ReadyToDrinkFilter], wxSizerFlags().Border(wxALL));

      // categorize controls so we can show/hide as appropriate.
      m_categorized.addControlDependency(ControlCategory::InStockFilter,      m_filter_checkboxes[InStockFilter ]    );
      m_categorized.addControlDependency(ControlCategory::MinScoreFilter,     m_filter_checkboxes[MinScoreFilter]    );
      m_categorized.addControlDependency(ControlCategory::MinScoreFilter,     m_score_spin_ctrl                      );
      m_categorized.addControlDependency(ControlCategory::ReadyToDrinkFilter, m_filter_checkboxes[ReadyToDrinkFilter]);
   }


   auto DatasetOptionsPanel::setTitle() -> bool
   {
      auto dataset = m_sink.getDatasetOrThrow();
      m_dataset_title->SetLabelText( dataset->getCollectionName());
      forceLayoutUpdate(this);
      return true;
   }


   auto DatasetOptionsPanel::getSortOptionList(IDataset* dataset) -> wxArrayString
   {
      return vws::all(dataset->availableSorts()) 
         | vws::transform([](const IDataset::TableSort& s) {  return wxFromSV(s.sort_name); })
         | rng::to<wxArrayString>();
   }

   
   void DatasetOptionsPanel::notify(DatasetEvent event)
   {
      assert(event.dataset);

      try
      {
         switch (event.event_id)
         {
         case DatasetEvent::Id::DatasetInitialize:
            onDatasetInitialize(event.dataset.get());
            break;

         case DatasetEvent::Id::Sort:
            onTableSorted(event.dataset.get());
            break;

         case DatasetEvent::Id::Filter:              [[fallthrough]];
         case DatasetEvent::Id::SubStringFilter:     [[fallthrough]];
         case DatasetEvent::Id::RowSelected:         [[fallthrough]];
         case DatasetEvent::Id::ColLayoutRequested:  [[fallthrough]];
         default:
            break;
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onDatasetInitialize(IDataset* dataset)
   {
      // reload sort/filter options
      m_sort_combo->Clear();
      m_sort_combo->Append(getSortOptionList(dataset));
      onTableSorted(dataset);
      setTitle();

      // show/hide/initialize filter checkboxes
      m_categorized.showCategory(ControlCategory::InStockFilter,      dataset->hasProperty(CtProp::QtyTotal     ));
      m_categorized.showCategory(ControlCategory::MinScoreFilter,     dataset->hasProperty(CtProp::CtScore      ));
      m_categorized.showCategory(ControlCategory::ReadyToDrinkFilter, dataset->hasProperty(CtProp::RtdQtyDefault));
      for (auto* check_box : vws::values(m_filter_checkboxes))
      {
         auto&& filter = dataset->propFilters().getFilter(check_box->filter().filter_name);
         check_box->enable(filter.has_value() ? true : false);
      }
      
      TransferDataToWindow();
      forceLayoutUpdate(this);
   }


   void DatasetOptionsPanel::onTableSorted(IDataset* dataset)
   {
      // need to update the index our combo is bound to in addition to the sort.
      m_sort_config = dataset->activeSort();
      
      auto sorts = dataset->availableSorts();
      for (const auto&& [idx, sort] : vws::enumerate(sorts))
      {
         if (sort.sort_name == m_sort_config.sort_name)
         {
            m_sort_selection = idx;
         }
      }

      m_sort_ascending = !m_sort_config.reverse;
      m_sort_descending = m_sort_config.reverse;
      TransferDataToWindow();
   }


   void DatasetOptionsPanel::onClearFilters([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         auto&& dataset = m_sink.getDatasetOrThrow();
         dataset->multivalFilters().clear();
         dataset->propFilters().clear();
         m_sink.signal_source(DatasetEvent::Id::DatasetInitialize);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onFilterChecked(ControlCategory control_cat)
   {
      try
      {
         TransferDataFromWindow();

         auto* checkbox = m_filter_checkboxes[control_cat];
         auto& filter = checkbox->filter();
         auto&& dataset = m_sink.getDatasetOrThrow();
         if (checkbox->enabled())
         {
            dataset->propFilters().replaceFilter(filter.filter_name, filter);
         }
         else {
            dataset->propFilters().disableFilterMatchValue(filter.filter_name);
         }
         m_sink.signal_source(DatasetEvent::Id::Filter);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onFilterInStockChecked([[maybe_unused]] wxCommandEvent& event)
   {
      onFilterChecked(ControlCategory::InStockFilter);
   }


   void DatasetOptionsPanel::onFilterMinScoreChecked([[maybe_unused]] wxCommandEvent& event)
   {
      onFilterChecked(ControlCategory::MinScoreFilter);
   }


   void DatasetOptionsPanel::onFilterReadyToDrinkChecked([[maybe_unused]] wxCommandEvent& event)
   {
      onFilterChecked(ControlCategory::ReadyToDrinkFilter);
   }


   void DatasetOptionsPanel::onMinScoreChanged([[maybe_unused]] wxSpinDoubleEvent& event)
   {
      try
      {
         auto dataset = m_sink.getDatasetOrThrow();
         TransferDataFromWindow();

         auto* checkbox = m_filter_checkboxes[ControlCategory::MinScoreFilter];
         auto& filter = checkbox->filter();
         filter.compare_val = event.GetValue();
         if (checkbox->enabled())
         {
            dataset->propFilters().replaceFilter(filter.filter_name, filter);
            m_sink.signal_source(DatasetEvent::Id::Filter);
         }
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onMinScoreUpdateUI(wxUpdateUIEvent& event)
   {
      TransferDataFromWindow();
      event.Enable(m_filter_checkboxes[ControlCategory::MinScoreFilter]->enabled());
   }


   void DatasetOptionsPanel::onSaveDefault([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         auto dataset = m_sink.getDatasetOrThrow();
         if (nullptr == dataset)
            throw Error{ constants::ERROR_STR_NO_DATASET, Error::Category::DataError };

         CtDatasetOptions::saveDefaultOptions(CtDatasetOptions::retrieveOptions(dataset));
         wxGetApp().displayInfoMessage(constants::FMT_DEFAULT_OPTIONS_SAVED_MSG, getTableDescription(dataset->getTableId()));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         m_sort_config.reverse = m_sort_descending;

         auto dataset = m_sink.getDatasetOrThrow();
         dataset->applySort(m_sort_config);
         m_sink.signal_source(DatasetEvent::Id::Sort);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onSortSelection([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         // event could get generated even if they didn't change the selection, don't waste our time.
         auto old_index = m_sort_selection;
         TransferDataFromWindow();
         if (old_index == m_sort_selection)
            return;

         // let the combo close its list before we reload the dataset
         CallAfter([this](){
            auto dataset = m_sink.getDatasetOrThrow();
            assert(m_sort_selection <= std::ssize(dataset->availableSorts()));

            // we re-fetch sorter based on index, because when a sort is selected from the combo
            // we want to use the default order for that sort, not whatever the current
            // selection is (e.g. Scores sort is descending by default).
            m_sort_config = dataset->availableSorts()[static_cast<size_t>(m_sort_selection)];
            dataset->applySort(m_sort_config);
            m_sink.signal_source(DatasetEvent::Id::Sort);
         });
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }

} // namespace ctb::app