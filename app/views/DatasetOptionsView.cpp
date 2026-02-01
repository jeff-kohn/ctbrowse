/*******************************************************************
 * @file DatasetOptionsView.cpp
 *
 * @brief implementation file for the DatasetOptionsView class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "controls/CheckBoxFilterCtrl.h" 
#include "controls/MultiValueFilterTreeCtrl.h"
#include "controls/SortOptionsPanel.h"
#include "controls/SpinDoubleFilterCtrl.h"
#include "views/DatasetOptionsView.h"
#include "wx_helpers.h"

#include <ctb/model/ScopedDatasetFreeze.h>
#include <ctb/utility_chrono.h>

#include <wx/button.h>
#include <wx/choice.h>
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
   

   [[nodiscard]] auto DatasetOptionsView::create(wxWindow* parent, const DatasetEventSourcePtr& source) noexcept(false) -> DatasetOptionsView*
   {
      if (!parent)
      {
         assert("parent pointer cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<DatasetOptionsView> wnd{ new DatasetOptionsView{source} };
      wnd->createWindow(parent);
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   DatasetOptionsView::DatasetOptionsView(const DatasetEventSourcePtr& source) : m_dataset_events{ source }
   {}


   void DatasetOptionsView::createWindow(wxWindow* parent)
   {
      using namespace ctb::constants;

      const auto default_border = wxSizerFlags::GetDefaultBorder();

      if (!Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

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
      top_sizer->Add(SortOptionsPanel::create(this, m_dataset_events.getSource()), wxSizerFlags{}.Expand().Border(wxALL));
      
      // Match filter options box, contains filter tree 
      auto* match_filters_box = new wxStaticBoxSizer(wxVERTICAL, this, LBL_MATCH_FILTERS);
      m_filter_tree = MultiValueFilterTreeCtrl::create(match_filters_box->GetStaticBox(), m_dataset_events.getSource());
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
      m_dataset_events.setDefaultHandler([this](const DatasetEvent& event) { onDatasetEvent(event);  });
   }


   void DatasetOptionsView::createOptionFilters(wxStaticBoxSizer* parent)
   {
      using enum CtProp;
      using namespace ctb::constants;

      auto&& dataset = m_dataset_events.getDataset(true);

      // ready-to-drink filter, matches if any formula  besides "fast" calculates RTD >= 0, only shows for RTD view
      if (dataset->getTableId() == TableId::Availability)
      {
         auto props = { RtdQtyDefault, RtdQtyLinear, RtdQtyBellCurve, RtdQtyEarlyCurve, RtdQtyLateCurve, RtdQtyFastMaturing, RtdQtyEarlyAndLate, RtdQtyBottlesPerYear, };
         CtPropertyFilter filter{ LBL_CHECK_READY_TO_DRINK, props, FILTER_AVAILABLE_MIN_QTY, CtPropFilterPredicate{ CtPredicateType::GreaterEqual } };
         parent->Add(CheckBoxFilterCtrl::create(parent->GetStaticBox(), m_dataset_events.getSource(), filter), wxSizerFlags().Border(wxALL));
         m_supported_filters.insert(filter.filter_name);
      }

      // in-stock filter
      if (dataset->hasProperty(QtyOnHand))
      {
         CtPropertyFilter filter{ LBL_CHECK_IN_STOCK_ONLY, { QtyOnHand }, uint16_t{0}, CtPropFilterPredicate{ CtPredicateType::Greater } };
         parent->Add(CheckBoxFilterCtrl::create(parent->GetStaticBox(), m_dataset_events.getSource(), filter), wxSizerFlags().Border(wxALL));
         m_supported_filters.insert(filter.filter_name);
      }

      // 'remaining bottles' filter
      if (dataset->hasProperty(PurchaseQtyRemaining))
      {
         CtPropertyFilter filter{ LBL_CHECK_WITH_REMAINING, { PurchaseQtyRemaining }, uint16_t{0}, CtPropFilterPredicate{ CtPredicateType::Greater } };
         parent->Add(CheckBoxFilterCtrl::create(parent->GetStaticBox(), m_dataset_events.getSource(), filter), wxSizerFlags().Border(wxALL));
         m_supported_filters.insert(filter.filter_name);
      }

      // min-score filter checkbox
      if (dataset->hasProperty(CtScore) or dataset->hasProperty(MyScore))
      {
         auto score_params = SpinDoubleFilterCtrl::SpinParams{
            .min_value = FILTER_SCORE_MIN,
            .max_value = FILTER_SCORE_MAX,
            .increment = FILTER_SCORE_INCR,
            .default_value = FILTER_SCORE_DEFAULT,
            .decimal_places = FILTER_SCORE_DIGITS,
         };
         CtPropertyFilter filter{ LBL_CHECK_MIN_SCORE, { CtScore, MyScore }, score_params.default_value, CtPropFilterPredicate{ CtPredicateType::GreaterEqual } };
         parent->Add(SpinDoubleFilterCtrl::create(parent->GetStaticBox(), m_dataset_events.getSource(), filter, score_params), wxSizerFlags{}.Expand().Border(wxALL));
         m_supported_filters.insert(filter.filter_name);
      }

      if (dataset->hasProperty(MyPrice))
      {
         // min price filter checkbox
         auto price_params = SpinDoubleFilterCtrl::SpinParams{
            .min_value = 0,
            .max_value = FILTER_PRICE_MAX,
            .increment = FILTER_PRICE_INCREMENT,
            .default_value = FILTER_MIN_PRICE_DEFAULT,
            .decimal_places = 0,
         };
         CtPropertyFilter filter{ LBL_CHECK_MIN_PRICE, { MyPrice }, price_params.default_value, CtPropFilterPredicate{ CtPredicateType::GreaterEqual } };
         parent->Add(SpinDoubleFilterCtrl::create(parent->GetStaticBox(), m_dataset_events.getSource(), filter, price_params), wxSizerFlags{}.Expand().Border(wxALL));
         m_supported_filters.insert(filter.filter_name);

         // max price filter checkbox
         price_params.default_value = FILTER_MAX_PRICE_DEFAULT;
         filter.compare_pred        = CtPropFilterPredicate{ CtPredicateType::LessEqual };
         filter.compare_val         = price_params.default_value;
         filter.filter_name         = LBL_CHECK_MAX_PRICE;

         parent->Add(SpinDoubleFilterCtrl::create(parent->GetStaticBox(), m_dataset_events.getSource(), filter, price_params), wxSizerFlags{}.Expand().Border(wxALL));
		   m_supported_filters.insert(filter.filter_name);
      }
   }


   auto DatasetOptionsView::setTitle() -> bool
   {
      auto dataset = m_dataset_events.getDataset(true);
      m_dataset_title->SetLabelText( dataset->getCollectionName());
      forceLayoutUpdate(this);
      return true;
   }


   void DatasetOptionsView::onDatasetEvent(DatasetEvent event)
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
            //onTableSorted(event.dataset);
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


   void DatasetOptionsView::onDatasetInitialize(DatasetPtr dataset)
   {
      setTitle();

      // For any property filters that we don't have UI for, we need to remove them from the dataset. Shouldn't happen 
      // but might in the case of filters persisted to file from an earlier version.
      {
         ScopedDatasetFreeze freeze{ dataset };
         auto active_filter_names = vws::keys(dataset->propFilters().activeFilters()) | rng::to<StringSet>();
         for (const auto& name : active_filter_names)
         {
            if (!m_supported_filters.contains(name))
            {
               wxGetApp().displayFormattedMessage("Removing unsupported filter '{}'", name);
               dataset->propFilters().removeFilter(name);
            }
         }
      }
     
      TransferDataToWindow();
      forceLayoutUpdate(this);
   }

} // namespace ctb::app