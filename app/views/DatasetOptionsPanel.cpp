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


#include <memory>

namespace ctb::app
{
   namespace 
   {
      constexpr int IMG_CONTAINER = 0;
      constexpr int IMG_UNCHECKED = 1;
      constexpr int IMG_CHECKED = 2;


      auto getPropertyForFieldType(const CtFieldSchema& fld, std::string_view text_val) -> CtProperty
      {
         switch (fld.prop_type)
         {
            case PropType::String:
               return CtProperty{ std::string{ text_val } };

            case PropType::UInt16:
               return CtProperty::create<uint16_t>(text_val);

            case PropType::UInt64:
               return CtProperty::create<uint64_t>(text_val);

            case PropType::Double:
               return CtProperty::create<double>(text_val);

            case PropType::Date:
            {
               auto ymd = parseDate(constants::FMT_PARSE_DATE_SHORT);
               return ymd ? CtProperty{ *ymd } : CtProperty{};
            }
            default:
               log::info("getPropertyForFieldType() encountered unexpected property type with value {}", std::to_underlying(fld.prop_type));
               assert("Unexpected property type, this is a bug" and false);
               return {};
         }
      }

   } // namespace
   

   DatasetOptionsPanel::DatasetOptionsPanel(DatasetEventSourcePtr source) : m_sink{ this, source }
   {}


   [[nodiscard]] auto DatasetOptionsPanel::create(wxWindow* parent, DatasetEventSourcePtr source) -> DatasetOptionsPanel*
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


   void DatasetOptionsPanel::initControls()
   {
      const auto default_border = wxSizerFlags::GetDefaultBorder();

      // panel shouldn't grow infinitely
      SetMaxSize(ConvertDialogToPixels(wxSize{ 150, constants::WX_UNSPECIFIED_VALUE }));
      SetMinSize(ConvertDialogToPixels(wxSize{ 100, constants::WX_UNSPECIFIED_VALUE }));

      // defines the rows of controls in our panel
      m_top_sizer = new wxBoxSizer{ wxVERTICAL };
      m_top_sizer->AddSpacer(default_border);

      // Dataset title
      auto title_font{ GetFont().MakeLarger().MakeBold()};
      const auto heading_color = wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT);
      constexpr auto title_border_size = 10;
      m_dataset_title = new wxStaticText{ this, wxID_ANY, "" };
      m_dataset_title->SetFont(title_font);
      m_dataset_title->SetForegroundColour(heading_color);
      m_top_sizer->Add(m_dataset_title, wxSizerFlags{}.Expand().Border(wxALL, title_border_size));

      // sort options box
      auto* sort_options_box = new wxStaticBoxSizer(wxVERTICAL, this, constants::LBL_SORT_OPTIONS);

      // sort fields combo
      m_sort_combo = new wxChoice(sort_options_box->GetStaticBox(), wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_selection));
      sort_options_box->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxALL));

      // ascending sort order radio. 
      auto opt_ascending = new wxRadioButton{
         sort_options_box->GetStaticBox(),
         wxID_ANY, 
         constants::LBL_SORT_ASCENDING,
         wxDefaultPosition, 
         wxDefaultSize, 
         wxRB_GROUP
      };
      opt_ascending->SetValue(true);
      opt_ascending->SetValidator(wxGenericValidator{ &m_sort_ascending });
      sort_options_box->Add(opt_ascending, wxSizerFlags{}.Expand().Border(wxALL));

      // descending sort order radio. Since the radio buttons aren't in a group box, the validator treats them as bool
      // so we have a separate flag for the descending radio that we have to manually keep in sync (see onTableSorted)
      auto opt_descending = new wxRadioButton{ sort_options_box->GetStaticBox(), wxID_ANY, constants::LBL_SORT_DESCENDING };
      opt_descending->SetValidator(wxGenericValidator{ &m_sort_descending });
      sort_options_box->Add(opt_descending, wxSizerFlags{1}.Expand().Border(wxALL));
      m_top_sizer->Add(sort_options_box, wxSizerFlags().Expand().Border(wxALL));
      m_top_sizer->AddSpacer(default_border);

      // filter options box
      m_filter_options_box = new wxStaticBoxSizer(wxVERTICAL, this, constants::LBL_FILTER_OPTIONS);

      // load images for the checkboxes in our filter tree.
      const auto tr_img_size = wxSize{ 16, 16 };
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(constants::RES_NAME_TREE_FILTER_IMG, tr_img_size));
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(constants::RES_NAME_TREE_UNCHECKED_IMG, tr_img_size));
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(constants::RES_NAME_TREE_CHECKED_IMG, tr_img_size));

      // filter tree control
      auto style = wxTR_DEFAULT_STYLE | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT;
      m_filter_tree = new wxTreeCtrl{ m_filter_options_box->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, style };
      m_filter_tree->SetMaxSize(ConvertDialogToPixels(wxSize(-1, 500)));
      m_filter_tree->SetMinSize(ConvertDialogToPixels(wxSize(-1, 100)));
      m_filter_tree->SetImages(m_filter_tree_images);
      m_filter_options_box->Add(m_filter_tree, wxSizerFlags(2).Expand().Border(wxALL));
      m_filter_options_box->AddSpacer(default_border);

      // in-stock filter
      auto instock_filter_ctrl = new wxCheckBox(m_filter_options_box->GetStaticBox(), wxID_ANY, constants::LBL_CHECK_IN_STOCK_ONLY);
      instock_filter_ctrl->SetValidator(wxGenericValidator{ &m_instock_only });
      m_filter_options_box->Add(instock_filter_ctrl, wxSizerFlags().Border(wxALL));

      // min-score filter sizer
      auto* min_score_sizer = new wxBoxSizer(wxHORIZONTAL);

      // enable score filter checkbox
      auto* score_filter_chk = new wxCheckBox{ m_filter_options_box->GetStaticBox(), wxID_ANY, constants::LBL_REQUIRE_MIN_SCORE };
      score_filter_chk->SetValidator(wxGenericValidator{ &m_enable_score_filter });
      min_score_sizer->Add(score_filter_chk, wxSizerFlags{}.Center().Border(wxLEFT|wxTOP|wxBOTTOM));

      // score filter value spinbox
      m_score_spin_ctrl = new wxSpinCtrlDouble
         {
            m_filter_options_box->GetStaticBox(), 
            wxID_ANY,
            wxEmptyString, wxDefaultPosition, wxDefaultSize, 
            wxSP_ARROW_KEYS|wxALIGN_RIGHT, 
            constants::FILTER_SCORE_MIN, constants::FILTER_SCORE_MAX, 
            constants::FILTER_SCORE_DEFAULT, constants::FILTER_SCORE_INCR
         };
      m_score_spin_ctrl->SetDigits(constants::FILTER_SCORE_DIGITS);
      m_score_spin_ctrl->Enable(false);
      min_score_sizer->Add(m_score_spin_ctrl, wxSizerFlags{}.Border(wxRIGHT|wxTOP|wxBOTTOM));
      m_filter_options_box->Add(min_score_sizer, wxSizerFlags());

      // finalize layout
      m_top_sizer->Add(m_filter_options_box, wxSizerFlags(1).Expand().Border(wxALL));
      m_top_sizer->AddStretchSpacer(2);
      SetSizer(m_top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &DatasetOptionsPanel::onSortSelection, this);
      m_filter_tree->Bind(wxEVT_TREE_ITEM_EXPANDING, &DatasetOptionsPanel::onTreeFilterExpanding, this);
      m_filter_tree->Bind(wxEVT_LEFT_DOWN, &DatasetOptionsPanel::onTreeFilterLeftClick, this);
      m_score_spin_ctrl->Bind(wxEVT_SPINCTRLDOUBLE, &DatasetOptionsPanel::onMinScoreChanged, this);
      opt_ascending->Bind(wxEVT_RADIOBUTTON, &DatasetOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &DatasetOptionsPanel::onSortOrderClicked, this);
      instock_filter_ctrl->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onInStockChecked, this);
      score_filter_chk->Bind(wxEVT_CHECKBOX, &DatasetOptionsPanel::onMinScoreFilterChecked, this);     
   }

   auto DatasetOptionsPanel::setTitle() -> bool
   {
      auto dataset = m_sink.getDataset();
      if (!dataset)
         return false;

      m_dataset_title->SetLabelText( wxFromSV(getTableDescription(dataset->getTableId())) );
      GetSizer()->Layout();
      SendSizeEvent();
      Update();
      return true;
   }


   void DatasetOptionsPanel::addPropFilter(wxTreeItemId item)
   {
      if (m_sink.hasDataset())
      {
         CtProperty filter_val{};
         auto filter = getPropFilterForItem(item);
         if (filter)
         {
            /// We need to convert the string value from the filter tree to the correct type, which may not be string.
            auto fld_schema = m_sink.getDataset()->getFieldSchema(filter->prop_id);
            if (fld_schema)
            {
               auto wx_str_val = m_filter_tree->GetItemText(item);
               std::string_view text_val{ wx_str_val.wx_str() };
               filter_val = getPropertyForFieldType(*fld_schema, text_val);
            }
            else {
               assert("Not getting a valid FieldSchema here is a bug." and false);
            }

            m_sink.getDataset()->addMultiMatchFilter(filter->prop_id, filter_val);
            m_sink.signal_source(DatasetEvent::Id::Filter);
         }
      }
   }


   void DatasetOptionsPanel::removePropFilter(wxTreeItemId item)
   {
      if (m_sink.hasDataset())
      {
         CtProperty filter_val{};
         auto filter = getPropFilterForItem(item);
         if (filter)
         {
            /// We need to convert the string value from the filter tree to the correct type, which may not be string.
            auto fld_schema = m_sink.getDataset()->getFieldSchema(filter->prop_id);
            if (fld_schema)
            {
               auto wx_str_val = m_filter_tree->GetItemText(item);
               std::string_view text_val{ wx_str_val.wx_str() };
               filter_val = getPropertyForFieldType(*fld_schema, text_val);
            }
            else {
               assert("Not getting a valid FieldSchema here is a bug." and false);
            }
            m_sink.getDataset()->removeMultiMatchFilter(filter->prop_id, filter_val);
            m_sink.signal_source(DatasetEvent::Id::Filter);
         }
      }
   }


   void DatasetOptionsPanel::populateFilterTypes(IDataset* dataset)
   {
      assert(m_filter_tree);

      // disable window updates till we're done and reset the tree
      wxWindowUpdateLocker freeze_updates{m_filter_tree};
      m_filter_tree->DeleteAllItems();
      m_filters.clear();
      m_check_map.clear();

      // get the available filters for this dataset, and add them to the tree.
      auto filters = dataset->multiMatchFilters();
      auto root = m_filter_tree->AddRoot(wxEmptyString);
      for (auto& filter : filters)
      {
         wxString filter_name{ wxString::FromUTF8(filter.filter_name) };
         auto item = m_filter_tree->AppendItem(root, filter_name);
         m_filter_tree->SetItemHasChildren(item, true);
         m_filter_tree->SetItemImage(item, IMG_CONTAINER);
         m_filters[item] = filter;
      }
   }


   auto DatasetOptionsPanel::getPropFilterForItem(wxTreeItemId item) -> MaybeFilter
   {
      auto dataset = m_sink.getDataset();
      if (dataset)
      {
         // we need to get the parent node's item, since that's what's in the map.
         auto parent = m_filter_tree->GetItemParent(item);
         if (parent.IsOk())
         {
            auto it = m_filters.find(parent);
            if (it != m_filters.end())
               return it->second;
         }
      }
      return {};
   }


   auto DatasetOptionsPanel::getSortOptionList(IDataset* dataset) -> wxArrayString
   {
      return vws::all(dataset->availableSorts()) 
               | vws::transform([](const IDataset::TableSort& s) {  return wxFromSV(s.sort_name); })
               | rng::to<wxArrayString>();
   }


   auto DatasetOptionsPanel::isChecked(wxTreeItemId item) -> bool
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CHECKED;
   }


   auto DatasetOptionsPanel::isContainerNode(wxTreeItemId item) -> bool
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CONTAINER;
   }


   auto DatasetOptionsPanel::isMatchValueNode(wxTreeItemId item) -> bool
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) != IMG_CONTAINER;
   }

   /// @brief updates the checked/unchecked status of a node.
   /// @return true if successful, false otherwise (ie invalid item)
   /// 
   auto DatasetOptionsPanel::setMatchValueChecked(wxTreeItemId item, bool checked) -> bool
   {
      if ( isMatchValueNode(item) )
      {
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
      bool checked = !isChecked(item);
      if (!setMatchValueChecked(item, checked))
         return;

      if (checked)
      {
         addPropFilter(item);
         m_check_map[item]++;
      }
      else{
         removePropFilter(item);
         m_check_map[item]--;
      }
      updateFilterLabel(m_filter_tree->GetItemParent(item));
   }


   void DatasetOptionsPanel::updateFilterLabel(wxTreeItemId item)
   {
      auto maybe_filter = m_filters[item];
      if ( !maybe_filter or !item.IsOk() )
         return;

      // if the filter node has selected children, update the label with the count
      wxString filter_name{ wxFromSV(maybe_filter->filter_name) };
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


   void DatasetOptionsPanel::enableInStockFilter(bool enable)
   {
      constexpr size_t index = 2;
      m_filter_options_box->Show(index, enable);
      m_filter_options_box->Layout();
      SendSizeEvent();
      Update();
   }


   void DatasetOptionsPanel::resetInStockCheckbox()
   {
      m_instock_only = false;
      TransferDataToWindow();
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
            enableInStockFilter(event.dataset->hasProperty(CtProp::QtyOnHand));
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

      m_instock_only = dataset->getInStockFilter();
      TransferDataToWindow();
   }


   void DatasetOptionsPanel::onTableSorted(IDataset* dataset)
   {
      m_sort_config = dataset->activeSort();
      m_sort_ascending = !m_sort_config.reverse;
      m_sort_descending = m_sort_config.reverse;
      TransferDataToWindow();
   }


   void DatasetOptionsPanel::onInStockChecked([[maybe_unused]] wxCommandEvent& event)
   {
      assert(m_sink.hasDataset());

      TransferDataFromWindow();
      if (m_sink.hasDataset() and m_sink.getDataset()->setInStockFilter(m_instock_only))
      {
         m_sink.signal_source(DatasetEvent::Id::Filter);
      }
      else{
         // something went wrong, so clear checkbox.
         m_instock_only = false;
         TransferDataToWindow();
      }
   }


   void DatasetOptionsPanel::onMinScoreChanged([[maybe_unused]] wxSpinDoubleEvent& event)
   {
      if (m_enable_score_filter and m_sink.hasDataset())
      {
         if ( m_sink.getDataset()->setMinScoreFilter(event.GetValue()) )
         {
            m_sink.signal_source(DatasetEvent::Id::Filter);
         }
      }
   }


   void DatasetOptionsPanel::onMinScoreFilterChecked([[maybe_unused]] wxCommandEvent& event)
   {
      if (!m_sink.hasDataset())
      {
         assert(false);
         return;
      }

      TransferDataFromWindow();
      m_score_spin_ctrl->Enable(m_enable_score_filter);
      if (m_enable_score_filter) 
      {
         if (m_sink.getDataset()->setMinScoreFilter(m_score_filter_val))
         {
            m_sink.signal_source(DatasetEvent::Id::Filter);
         }
      }
      else 
      {
         if (m_sink.getDataset()->setMinScoreFilter(std::nullopt))
         {
            m_sink.signal_source(DatasetEvent::Id::Filter);
         }
      }
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
               // we re-fetch sorter based on index, because when a sort is selected from the combo
               // we want to use the default order for that sort, not necessarily whatever the current
               // selection is (e.g. sort Scores descending by default).
               auto sorts = dataset->availableSorts();
               m_sort_config = sorts[static_cast<size_t>(m_sort_selection)];
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
         if (!filter_node.IsOk())
            return;

         // if the node has a filter in our map and it doesn't already have a list of 
         // available filter values as children, we need to populate the child nodes.
         // otherwise just return.
         if( !m_filters.contains(filter_node.GetID()) || m_filter_tree->GetChildrenCount(filter_node) > 0 )
         {
            return;
         }

         auto data = m_sink.getDataset();
         assert(data);

         auto& filter = m_filters[filter_node.m_pItem]; 
         for (auto& match_val : data->getDistinctValues(filter->prop_id))
         {
            auto item = m_filter_tree->AppendItem(filter_node, wxString::FromUTF8(match_val.asString()));
            m_filter_tree->SetItemImage(item, 1);
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