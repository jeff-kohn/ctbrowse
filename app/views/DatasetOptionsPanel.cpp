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
#include "views/SpinDoubleFilterCtrl.h"
#include "model/CtDatasetOptions.h"
#include "wx_helpers.h"

#include <ctb/model/ScopedDatasetFreeze.h>
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
#include <wx/slider.h>

#include <string>
#include <string_view>

namespace ctb::app
{
   namespace 
   {

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
      
      // Match filter options box, contains filter tree 
      auto* match_filters_box = new wxStaticBoxSizer(wxVERTICAL, this, LBL_MATCH_FILTERS);
      m_filter_tree = MultiValueFilterTree::create(*match_filters_box->GetStaticBox(), m_sink.getSource());
      m_filter_tree->SetMaxSize(ConvertDialogToPixels(wxSize(-1, 500)));
      m_filter_tree->SetMinSize(ConvertDialogToPixels(wxSize(-1, 100)));
      match_filters_box->Add(m_filter_tree, wxSizerFlags(2).Expand().Border(wxALL));
      match_filters_box->AddSpacer(default_border);
      top_sizer->Add(match_filters_box, wxSizerFlags(1).Expand().Border(wxALL));

      // checkbox option filters
      auto* option_filters_box = new wxStaticBoxSizer(wxVERTICAL, this, LBL_OPTION_FILTERS);
      createOptionFilters(option_filters_box);
      top_sizer->Add(option_filters_box, wxSizerFlags{}.Expand().Border(wxALL));

      // finalize layout
      top_sizer->AddStretchSpacer(2);
      SetSizer(top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &DatasetOptionsPanel::onSortSelection, this);
      opt_ascending->Bind( wxEVT_RADIOBUTTON,   &DatasetOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON,   &DatasetOptionsPanel::onSortOrderClicked, this);
      m_filter_checkboxes[ControlCategory::InStockFilter      ]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterInStockChecked,       this);
      m_filter_checkboxes[ControlCategory::ReadyToDrinkFilter ]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterReadyToDrinkChecked,  this);     
      m_filter_checkboxes[ControlCategory::WithRemainingFilter]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterWithRemainingChecked, this);
   }


   void DatasetOptionsPanel::createOptionFilters(wxStaticBoxSizer* parent)
   {
      using enum CtProp;
      using enum ControlCategory;
      using namespace ctb::constants;

      // ready-to-drink filter, matches if any formula  besides "fast" calculates RTD >= 0, only shows for RTD view
      auto props = { RtdQtyDefault, RtdQtyLinear, RtdQtyBellCurve, RtdQtyEarlyCurve, RtdQtyLateCurve, RtdQtyFastMaturing, RtdQtyEarlyAndLate, RtdQtyBottlesPerYear, };
      m_filter_checkboxes[ReadyToDrinkFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_READY_TO_DRINK, props, FILTER_AVAILABLE_MIN_QTY, CtPropFilterPredicate{ CtPredicateType::GreaterEqual } } };
      parent->Add(m_filter_checkboxes[ReadyToDrinkFilter], wxSizerFlags().Border(wxALL));
      parent->AddSpacer(4);

      // in-stock filter
      m_filter_checkboxes[InStockFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_IN_STOCK_ONLY, { QtyOnHand }, uint16_t{0}, CtPropFilterPredicate{ CtPredicateType::Greater } } };
      parent->Add(m_filter_checkboxes[InStockFilter], wxSizerFlags().Border(wxALL));
      parent->AddSpacer(2);


      // 'remaining bottles' filter
      m_filter_checkboxes[WithRemainingFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_WITH_REMAINING, { PurchaseQtyRemaining }, uint16_t{0}, CtPropFilterPredicate{ CtPredicateType::Greater } } };
      parent->Add(m_filter_checkboxes[WithRemainingFilter], wxSizerFlags().Border(wxALL));
      parent->AddSpacer(2);

      // min-score filter checkbox
      auto score_params = SpinDoubleFilterCtrl::SpinParams{
         .min_value = FILTER_SCORE_MIN, 
         .max_value = FILTER_SCORE_MAX,
         .increment = FILTER_SCORE_INCR,
         .default_value = FILTER_SCORE_DEFAULT,
         .decimal_places = FILTER_SCORE_DIGITS,
      };
      auto score_filter = CtPropertyFilter{
         LBL_CHECK_MIN_SCORE,
         { CtScore, MyScore },
         score_params.default_value,
         CtPropFilterPredicate{ CtPredicateType::GreaterEqual }
      };
      m_min_score_filter_ctrl = SpinDoubleFilterCtrl::create(*(parent->GetStaticBox()), m_sink.getSource(), score_filter, score_params);
      parent->Add(m_min_score_filter_ctrl, wxSizerFlags{}.Expand().Border(wxALL));

      // min price filter checkbox
      auto price_params = SpinDoubleFilterCtrl::SpinParams{
         .min_value      = 0, 
         .max_value      = FILTER_PRICE_MAX,
         .increment      = FILTER_PRICE_INCREMENT,
         .default_value  = FILTER_MIN_PRICE_DEFAULT,
         .decimal_places = 0,
      };
      auto price_filter = CtPropertyFilter{
         LBL_CHECK_MIN_PRICE,
         { MyPrice },
         price_params.default_value,
         CtPropFilterPredicate{ CtPredicateType::GreaterEqual }
      };
      m_min_price_filter_ctrl = SpinDoubleFilterCtrl::create(*(parent->GetStaticBox()), m_sink.getSource(), price_filter, price_params);
      parent->Add(m_min_price_filter_ctrl, wxSizerFlags{}.Expand().Border(wxALL));

      // max price filter checkbox
      price_params.default_value = FILTER_MAX_PRICE_DEFAULT;
      price_filter.compare_pred  = CtPropFilterPredicate{ CtPredicateType::LessEqual };
      price_filter.compare_val   = price_params.default_value;
      price_filter.filter_name   = LBL_CHECK_MAX_PRICE;
      m_max_price_filter_ctrl    = SpinDoubleFilterCtrl::create(*(parent->GetStaticBox()), m_sink.getSource(), price_filter, price_params);
      parent->Add(m_max_price_filter_ctrl, wxSizerFlags{}.Expand().Border(wxALL));

      // categorize controls so we can show/hide as appropriate.
      m_categorized.addControlDependency(ControlCategory::InStockFilter,       m_filter_checkboxes[InStockFilter ]     );
      m_categorized.addControlDependency(ControlCategory::MaxPriceFilter,      m_max_price_filter_ctrl                 );
      m_categorized.addControlDependency(ControlCategory::MinPriceFilter,      m_min_price_filter_ctrl                 );
      m_categorized.addControlDependency(ControlCategory::MinScoreFilter,      m_min_score_filter_ctrl                 );
      m_categorized.addControlDependency(ControlCategory::ReadyToDrinkFilter,  m_filter_checkboxes[ReadyToDrinkFilter] );
      m_categorized.addControlDependency(ControlCategory::WithRemainingFilter, m_filter_checkboxes[WithRemainingFilter]);

   }


   auto DatasetOptionsPanel::setTitle() -> bool
   {
      auto dataset = m_sink.getDatasetOrThrow();
      m_dataset_title->SetLabelText( dataset->getCollectionName());
      forceLayoutUpdate(this);
      return true;
   }


   auto DatasetOptionsPanel::getSortOptionList(DatasetPtr dataset) -> wxArrayString
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
         case DatasetEvent::Id::Filter:              [[fallthrough]];
         case DatasetEvent::Id::DatasetInitialize:
            onDatasetInitialize(event.dataset);
            break;

         case DatasetEvent::Id::Sort:
            onTableSorted(event.dataset);
            break;

         case DatasetEvent::Id::SubStringFilter:     [[fallthrough]];
         case DatasetEvent::Id::RowSelected:         [[fallthrough]];
         default:
            break;
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onDatasetInitialize(DatasetPtr dataset)
   {
      // reload sort/filter options
      m_sort_combo->Clear();
      m_sort_combo->Append(getSortOptionList(dataset));
      onTableSorted(dataset);
      setTitle();

      // show/hide/initialize filter checkboxes
      m_categorized.showCategory(ControlCategory::InStockFilter,       dataset->hasProperty(CtProp::QtyTotal));
      m_categorized.showCategory(ControlCategory::MaxPriceFilter,      dataset->hasProperty(CtProp::MyPrice));
      m_categorized.showCategory(ControlCategory::MinPriceFilter,      dataset->hasProperty(CtProp::MyPrice));
      m_categorized.showCategory(ControlCategory::MinScoreFilter,      dataset->hasProperty(CtProp::CtScore));
      m_categorized.showCategory(ControlCategory::ReadyToDrinkFilter,  dataset->hasProperty(CtProp::RtdQtyDefault));
      m_categorized.showCategory(ControlCategory::WithRemainingFilter, dataset->hasProperty(CtProp::PurchaseQtyRemaining));

		// keep track of which filters we have controls for
      StringSet matched_filter_names{};

      // get the filters for our checkboxes
      for (auto* check_box : vws::values(m_filter_checkboxes))
      {
         auto filter = dataset->propFilters().getFilter(check_box->filter().filter_name);
         if (filter)
         {
            matched_filter_names.insert(filter->filter_name);
            check_box->enable(true);
         }
         else {
				check_box->enable(false);
         }
      }

      // For any property filters that we don't have a matching control for, we need to remove them from the dataset
      {
         ScopedDatasetFreeze freeze{ dataset };
         auto active_filter_names = vws::keys(dataset->propFilters().activeFilters()) | rng::to<StringSet>();
         for (const auto& name : active_filter_names)
         {
            if (!matched_filter_names.contains(name))
            {
               wxGetApp().displayFormattedMessage("Removing unsupported filter '{}'", name);
               dataset->propFilters().removeFilter(name);
            }
         }
      }
      
      TransferDataToWindow();
      forceLayoutUpdate(this);
   }


   void DatasetOptionsPanel::onTableSorted(DatasetPtr dataset)
   {
      m_sort_config = dataset->activeSort();
      m_sort_ascending = (m_sort_config.reverse == false);
      m_sort_descending = m_sort_config.reverse;

      for (const auto&& [idx, sort] : vws::enumerate(dataset->availableSorts()))
      {
         if (m_sort_config.sort_name == sort.sort_name)
         {
            m_sort_selection = idx;
         }
      }

      TransferDataToWindow();
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
            dataset->propFilters().removeFilter(filter.filter_name);
         }
         m_sink.signal_source(DatasetEvent::Id::Filter, false);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onFilterInStockChecked([[maybe_unused]] wxCommandEvent& event)
   {
      onFilterChecked(ControlCategory::InStockFilter);
   }


   void DatasetOptionsPanel::onFilterReadyToDrinkChecked([[maybe_unused]] wxCommandEvent& event)
   {
      onFilterChecked(ControlCategory::ReadyToDrinkFilter);
   }


   void DatasetOptionsPanel::onFilterWithRemainingChecked([[maybe_unused]] wxCommandEvent& event)
   {
      onFilterChecked(ControlCategory::WithRemainingFilter);
   }


   void DatasetOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto dataset = m_sink.getDatasetOrThrow();
         m_sort_config.reverse = m_sort_descending;
         dataset->applySort(m_sort_config);
         m_sink.signal_source(DatasetEvent::Id::Sort, false);
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
         CallAfter([this]()
            {
               auto dataset = m_sink.getDatasetOrThrow();
               auto sorts = dataset->availableSorts();
               if (m_sort_selection <= std::ssize(sorts))
               {
                  // re-fetch sorter based on index. UI and member state will get updated in the event handler.
                  dataset->applySort(sorts[static_cast<size_t>(m_sort_selection)]);
                  m_sink.signal_source(DatasetEvent::Id::Sort, true); 
               }
               else {
						log::warn("DatasetOptionsPanel::onSortSelection: invalid sort index selected: {}", m_sort_selection);
               }
            });
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }

} // namespace ctb::app