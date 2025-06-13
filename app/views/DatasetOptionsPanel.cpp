/*******************************************************************
 * @file DatasetOptionsPanel.cpp
 *
 * @brief implementation file for the DatasetOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "App.h"
#include "wx_helpers.h"
#include "views/DatasetOptionsPanel.h"
#include "views/FilterCheckBox.h"

#include <ctb/utility_chrono.h>

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

      auto getPropertyForFieldType(const CtFieldSchema& fld, std::string_view text_val) -> CtPropertyVal
      {
         switch (fld.prop_type)
         {
            case PropType::String:
               return CtPropertyVal{ std::string{ text_val } };

            case PropType::UInt16:
               return CtPropertyVal::create<uint16_t>(text_val);

            case PropType::UInt64:
               return CtPropertyVal::create<uint64_t>(text_val);

            case PropType::Double:
               return CtPropertyVal::create<double>(text_val);

            case PropType::Date:
            {
               auto ymd = parseDate(text_val, constants::FMT_PARSE_DATE_SHORT);
               return ymd ? CtPropertyVal{ *ymd } : CtPropertyVal{};
            }
            default:
               log::info("getPropertyForFieldType() encountered unexpected property type with value {}", std::to_underlying(fld.prop_type));
               assert("Unexpected property type, this is a bug" and false);
               return {};
         }
      }

      void forceLayoutUpdate(wxWindow* window)
      {
         window->GetSizer()->Layout();
         window->SendSizeEvent();
         window->Update();
      }
   } // namespace
   

   [[nodiscard]] auto DatasetOptionsPanel::create(wxWindow* parent, DatasetEventSourcePtr source) noexcept(false) -> DatasetOptionsPanel*
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!parent)
      {
         assert("parent parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<DatasetOptionsPanel> wnd{ new DatasetOptionsPanel{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
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

      // load images for the checkboxes in our filter tree.
      const auto tr_img_size = wxSize{ 16, 16 };
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(RES_NAME_TREE_FILTER_IMG, tr_img_size));
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(RES_NAME_TREE_UNCHECKED_IMG, tr_img_size));
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(RES_NAME_TREE_CHECKED_IMG, tr_img_size));

      // filter tree control
      auto style = wxTR_DEFAULT_STYLE | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT;
      m_filter_tree = new wxTreeCtrl{ filter_options_box->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, style };
      m_filter_tree->SetMaxSize(ConvertDialogToPixels(wxSize(-1, 500)));
      m_filter_tree->SetMinSize(ConvertDialogToPixels(wxSize(-1, 100)));
      m_filter_tree->SetImages(m_filter_tree_images);
      filter_options_box->Add(m_filter_tree, wxSizerFlags(2).Expand().Border(wxALL));
      filter_options_box->AddSpacer(default_border);

      // checkbox filters
      createOptionFilters(filter_options_box);

      // finalize layout
      top_sizer->Add(filter_options_box, wxSizerFlags(1).Expand().Border(wxALL));
      top_sizer->AddStretchSpacer(2);
      SetSizer(top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &DatasetOptionsPanel::onSortSelection, this);
      m_filter_tree->Bind(wxEVT_TREE_ITEM_EXPANDING, &DatasetOptionsPanel::onTreeFilterExpanding, this);
      m_filter_tree->Bind(wxEVT_LEFT_DOWN,           &DatasetOptionsPanel::onTreeFilterLeftClick, this);

      m_score_spin_ctrl->Bind(wxEVT_SPINCTRLDOUBLE, &DatasetOptionsPanel::onMinScoreChanged,  this);
      m_score_spin_ctrl->Bind(wxEVT_UPDATE_UI,      &DatasetOptionsPanel::onMinScoreUpdateUI, this);    

      opt_ascending->Bind( wxEVT_RADIOBUTTON, &DatasetOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &DatasetOptionsPanel::onSortOrderClicked, this);

      m_filter_checkboxes[ControlCategory::InStockFilter     ]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterInStockChecked,        this);
      m_filter_checkboxes[ControlCategory::MinScoreFilter    ]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterMinScoreChecked, this);     
      m_filter_checkboxes[ControlCategory::ReadyToDrinkFilter]->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onFilterReadyToDrinkChecked,   this);     
   }


   void DatasetOptionsPanel::createOptionFilters(wxStaticBoxSizer* parent)
   {
      using enum CtProp;
      using enum ControlCategory;
      using namespace ctb::constants;

      // in-stock filter
      m_filter_checkboxes[InStockFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_IN_STOCK_ONLY, { QtyOnHand }, uint16_t{0}, std::greater<CtPropertyVal>{} } };
      parent->Add(m_filter_checkboxes[InStockFilter], wxSizerFlags().Border(wxALL));

      // embedded sizer to place score spin-ctrl next to checkbox
      auto* min_score_sizer = new wxBoxSizer(wxHORIZONTAL);

      // min-score filter checkbox
      m_filter_checkboxes[MinScoreFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_MIN_SCORE, { CtScore, MyScore }, FILTER_SCORE_DEFAULT, std::greater_equal<CtPropertyVal>{} } };
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
      m_filter_checkboxes[ReadyToDrinkFilter] = new FilterCheckBox{ *(parent->GetStaticBox()), { LBL_CHECK_READY_TO_DRINK, props, FILTER_AVAILABLE_MIN_QTY, std::greater<CtPropertyVal>{} }};
      parent->Add(m_filter_checkboxes[ReadyToDrinkFilter], wxSizerFlags().Border(wxALL));

      // categorize controls so we can show/hide as appropriate.
      m_categorized.addControlDependency(ControlCategory::InStockFilter,      m_filter_checkboxes[InStockFilter ]    );
      m_categorized.addControlDependency(ControlCategory::MinScoreFilter,     m_filter_checkboxes[MinScoreFilter]    );
      m_categorized.addControlDependency(ControlCategory::MinScoreFilter,     m_score_spin_ctrl                      );
      m_categorized.addControlDependency(ControlCategory::ReadyToDrinkFilter, m_filter_checkboxes[ReadyToDrinkFilter]);
   }


   auto DatasetOptionsPanel::setTitle() -> bool
   {
      auto dataset = m_sink.getDataset();
      if (!dataset)
         return false;

      m_dataset_title->SetLabelText( wxFromSV(getTableDescription(dataset->getTableId())) );
      forceLayoutUpdate(this);
      return true;
   }


   void DatasetOptionsPanel::addMultiValFilter(wxTreeItemId item) noexcept(false) 
   {
      if (m_sink.hasDataset())
      {
         auto& filter = getMultiValFilterForItem(item);

         if (auto fld_schema = m_sink.getDataset()->getFieldSchema(filter.prop_id); fld_schema.has_value() )
         {
            /// We need to convert the string value from the filter tree to the correct type, which may not be string.
            auto wx_str_val = m_filter_tree->GetItemText(item);
            std::string_view text_val{ wx_str_val.wx_str() };
            auto filter_val = getPropertyForFieldType(*fld_schema, text_val);
            m_sink.getDataset()->addMultiValueFilter(filter.prop_id, filter_val);
         }
         else {
            assert("Not getting a valid FieldSchema here is a bug." and false);
         }

         m_sink.signal_source(DatasetEvent::Id::Filter);
      }
      else {
         throw Error{ constants::ERROR_STR_NO_DATASET, Error::Category::DataError };
      }
   }
  


   auto DatasetOptionsPanel::getMultiValFilterForItem(wxTreeItemId item)  noexcept(false) -> CtMultiValueFilter&
   {
      auto dataset = m_sink.getDataset();
      if (dataset)
      {
         // we need to get the parent node's item, since that's what's in the map.
         auto parent = m_filter_tree->GetItemParent(item);
         if (parent.IsOk())
         {
            auto it = m_multival_filters.find(parent);
            if (it != m_multival_filters.end())
               return it->second;
         }
      }
      throw Error{ constants::ERROR_STR_FILTER_NOT_FOUND, Error::Category::DataError };
   }


   auto DatasetOptionsPanel::isItemChecked(wxTreeItemId item) -> bool
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CHECKED;
   }


   auto DatasetOptionsPanel::isItemContainerNode(wxTreeItemId item) -> bool
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CONTAINER;
   }


   auto DatasetOptionsPanel::isItemMatchValueNode(wxTreeItemId item) -> bool
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) != IMG_CONTAINER;
   }


   void DatasetOptionsPanel::removeMultiValFilter(wxTreeItemId item)  noexcept(false) 
   {
      if (!m_sink.hasDataset())
      {
         throw Error{ constants::ERROR_STR_NO_DATASET, Error::Category::DataError };
      }

      CtPropertyVal filter_val{};
      auto& filter = getMultiValFilterForItem(item);

      /// We need to convert the string value from the filter tree to the correct type, which may not be string.
      auto fld_schema = m_sink.getDataset()->getFieldSchema(filter.prop_id);
      if (fld_schema)
      {
         auto wx_str_val = m_filter_tree->GetItemText(item);
         std::string_view text_val{ wx_str_val.wx_str() };
         filter_val = getPropertyForFieldType(*fld_schema, text_val);
      }
      else {
         assert("Not getting a valid FieldSchema here is a bug." and false);
      }
      m_sink.getDataset()->removeMultiValueFilter(filter.prop_id, filter_val);
      m_sink.signal_source(DatasetEvent::Id::Filter);
   }


   /// @brief updates the checked/unchecked status of a node.
   /// @return true if successful, false otherwise (ie invalid item)
   auto DatasetOptionsPanel::setMultiValChecked(wxTreeItemId item, bool checked) -> bool
   {
      if ( isItemMatchValueNode(item) )
      {
         // update the image to reflect that it's checked, also update parent's
         // check count so we can display it in Filter Title.
         if (checked)
         {
            m_check_map[m_filter_tree->GetItemParent(item)]++;
            m_filter_tree->SetItemImage(item, IMG_CHECKED);
         }
         else{
            m_check_map[m_filter_tree->GetItemParent(item)]--;
            m_filter_tree->SetItemImage(item, IMG_UNCHECKED);
         }
         return true;
      }
      return false;
   }


   /// @brief toggles a filter value by updating its checked/unchecked image and applying/deleting the corresponding filter.
   /// 
   void DatasetOptionsPanel::toggleFilterSelection(wxTreeItemId item)
   {
      bool checked = isItemChecked(item) ? false : true; // toggle
      if (!setMultiValChecked(item, checked))
         return;

      if (checked)
      {
         addMultiValFilter(item);
      }
      else{
         removeMultiValFilter(item);
      }
      updateFilterLabel(m_filter_tree->GetItemParent(item));
   }


   void DatasetOptionsPanel::updateFilterLabel(wxTreeItemId item)
   {
      if ( !m_multival_filters.contains(item) or !item.IsOk() )
         return;

      auto& filter = m_multival_filters[item];

      // if the filter node has selected children, update the label with the count
      wxString filter_name{ filter.filter_name };
      auto count = m_check_map[item];
      if (count)
      {
         auto lbl = ctb::format(constants::FMT_LBL_FILTERS_SELECTED, filter_name.wx_str(), count);
         m_filter_tree->SetItemText(item, lbl.c_str());
      }
      else{
         m_filter_tree->SetItemText(item, filter_name);
      }
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
            onTableInitialize(event.dataset.get());
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


   void DatasetOptionsPanel::onTableInitialize(IDataset* dataset)
   {
      // reload sort/filter options
      m_sort_combo->Clear();
      m_sort_combo->Append(getSortOptionList(dataset));
      onTableSorted(dataset);
      populateFilterTypes(dataset);
      setTitle();

      // show/hide/initialize filter checkboxes
      m_categorized.showCategory(ControlCategory::InStockFilter,      dataset->hasProperty(CtProp::QtyTotal     ));
      m_categorized.showCategory(ControlCategory::MinScoreFilter,     dataset->hasProperty(CtProp::CtScore      ));
      m_categorized.showCategory(ControlCategory::ReadyToDrinkFilter, dataset->hasProperty(CtProp::RtdQtyDefault));
      for (auto* check_box : vws::values(m_filter_checkboxes))
      {
         auto&& filter = dataset->getPropFilter(check_box->filter().name);
         check_box->enable(filter.has_value() ? true : false);
      }
      
      TransferDataToWindow();
      forceLayoutUpdate(this);
   }


   void DatasetOptionsPanel::onTableSorted(IDataset* dataset)
   {
      m_sort_config = dataset->activeSort();
      m_sort_ascending = !m_sort_config.reverse;
      m_sort_descending = m_sort_config.reverse;
      TransferDataToWindow();
   }


   void DatasetOptionsPanel::populateFilterTypes(IDataset* dataset)
   {
      assert(m_filter_tree);

      // disable window updates till we're done and reset the tree
      wxWindowUpdateLocker freeze_updates{m_filter_tree};
      m_filter_tree->DeleteAllItems();
      m_multival_filters.clear();
      m_check_map.clear();

      // get the available filters for this dataset, and add them to the tree.
      auto filters = dataset->availableMultiValueFilters();
      auto root = m_filter_tree->AddRoot(wxEmptyString);
      for (auto& filter : filters)
      {
         wxString filter_name{ filter.filter_name };
         auto item = m_filter_tree->AppendItem(root, filter_name);
         m_filter_tree->SetItemHasChildren(item, true);
         m_filter_tree->SetItemImage(item, IMG_CONTAINER);
         m_multival_filters[item] = filter;
      }
   }


   void DatasetOptionsPanel::onFilterChecked(ControlCategory control_cat)
   {
      try
      {
         assert(m_sink.hasDataset());
         TransferDataFromWindow();

         auto* checkbox = m_filter_checkboxes[control_cat];
         auto& filter = checkbox->filter();
         if (checkbox->enabled())
         {
            m_sink.getDataset()->applyPropFilter(filter);
         }
         else {
            m_sink.getDataset()->removePropFilter(filter.name);
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
         assert(m_sink.hasDataset());
         TransferDataFromWindow();

         auto* checkbox = m_filter_checkboxes[ControlCategory::MinScoreFilter];
         auto& filter = checkbox->filter();
         filter.compare_val = event.GetValue();
         if (checkbox->enabled())
         {
            m_sink.getDataset()->applyPropFilter(filter);
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


   void DatasetOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto dataset = m_sink.getDataset();
         if (dataset)
         {
            m_sort_config.reverse = m_sort_descending;
            dataset->applySort(m_sort_config);
            m_sink.signal_source(DatasetEvent::Id::Sort);
         }
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
            auto dataset = m_sink.getDataset();
            if (dataset)
            {
               assert(m_sort_selection <= std::ssize(dataset->availableSorts()));

               // we re-fetch sorter based on index, because when a sort is selected from the combo
               // we want to use the default order for that sort, not whatever the current
               // selection is (e.g. sort Scores descending by default).
               m_sort_config = dataset->availableSorts()[static_cast<size_t>(m_sort_selection)];
               dataset->applySort(m_sort_config);
               m_sink.signal_source(DatasetEvent::Id::Sort);
            }
         });
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onTreeFilterExpanding(wxTreeEvent& event) 
   {
      try
      {
         auto filter_node = event.GetItem();
         auto dataset = m_sink.getDataset();
         if (!filter_node.IsOk() or !dataset)
         {
            assert("Something got corrupted, should never get invalid objects here" and false);
            throw Error{ constants::ERROR_STR_NO_DATASET, Error::Category::DataError };
         }

         // if the node has a filter in our map and it already has a list of 
         // available filter values as children, we don't need to do anything.
         if( !m_multival_filters.contains(filter_node.GetID()) || m_filter_tree->GetChildrenCount(filter_node) > 0 )
            return;

         auto createFilterNode = [&filter_node, this](const CtPropertyVal& match_value)
            {
               auto item = m_filter_tree->AppendItem(filter_node, match_value.asString());
               m_filter_tree->SetItemImage(item, IMG_UNCHECKED);
            };

         // depending on filter we may need to show the match values in descending order
         auto& filter = m_multival_filters[filter_node.m_pItem]; 
         if (filter.reverse_match_values)
         {
            rng::for_each(vws::reverse(dataset->getDistinctValues(filter.prop_id, false)), createFilterNode);
         }
         else {
            rng::for_each(dataset->getDistinctValues(filter.prop_id, false), createFilterNode);
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetOptionsPanel::onTreeFilterLeftClick(wxMouseEvent& event)
   {
      try
      {
         int flags{};
         auto item = m_filter_tree->HitTest(event.GetPosition(), flags);

         if ( item.IsOk() and (flags & wxTREE_HITTEST_ONITEMICON) )
         {
            toggleFilterSelection(item); // safe to call even if it's a filter/container node
         }
         else{
            event.Skip(); // need default processing for parent node's +/- button
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


} // namespace ctb::app