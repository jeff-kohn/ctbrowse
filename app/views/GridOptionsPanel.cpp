/*******************************************************************
 * @file GridOptionsPanel.cpp
 *
 * @brief implementation file for the GridOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "App.h"
#include "wx_helpers.h"
#include "views/GridOptionsPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wupdlock.h>
#include <wx/valgen.h>

#include <memory>

namespace ctb::app
{
   namespace 
   {
      constexpr int IMG_CONTAINER = 0;
      constexpr int IMG_UNCHECKED = 1;
      constexpr int IMG_CHECKED = 2;

   } // namespace
   
   GridOptionsPanel::GridOptionsPanel(GridTableEventSourcePtr source) : m_sink{ this, source }
   {
      auto& cfg = wxGetApp().getConfig();
      cfg.SetPath(constants::CONFIG_PATH_SYNC);
      cfg.Read(constants::CONFIG_VALUE_DEFAULT_IN_STOCK_ONLY, &m_instock_only, constants::CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT);
   }


   [[nodiscard]] GridOptionsPanel* GridOptionsPanel::create(wxWindow* parent, GridTableEventSourcePtr source)
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

      std::unique_ptr<GridOptionsPanel> wnd{ new GridOptionsPanel{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   void GridOptionsPanel::initControls()
   {
      auto default_border = wxSizerFlags::GetDefaultBorder();

      // panel shouldn't grow infinitely
      SetMaxSize(ConvertDialogToPixels(wxSize{ 150, constants::WX_UNSPECIFIED_VALUE }));
      SetMinSize(ConvertDialogToPixels(wxSize{ 100, constants::WX_UNSPECIFIED_VALUE }));

      // defines the rows of controls in our panel
      auto top_sizer = new wxBoxSizer{ wxVERTICAL };
      top_sizer->AddSpacer(default_border);

      // sort options box
      auto* sort_options_box = new wxStaticBoxSizer(wxVERTICAL, this, constants::LBL_SORT_OPTIONS);

      // sort fields combo
      m_sort_combo = new wxChoice(sort_options_box->GetStaticBox(), wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_config.sort_index));
      sort_options_box->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxALL));

      // ascending sort order radio. validator tied to GridTableSortConfig.ascending
      auto opt_ascending = new wxRadioButton{
         sort_options_box->GetStaticBox(),
         wxID_ANY, 
         constants::LBL_SORT_ASCENDING,
         wxDefaultPosition, 
         wxDefaultSize, 
         wxRB_GROUP
      };
      opt_ascending->SetValue(true);
      opt_ascending->SetValidator(wxGenericValidator{ &m_sort_config.ascending });
      sort_options_box->Add(opt_ascending, wxSizerFlags{}.Expand().Border(wxALL));

      // descending sort order radio. no validator needed 
      auto opt_descending = new wxRadioButton{ sort_options_box->GetStaticBox(), wxID_ANY, constants::LBL_SORT_DESCENDING };
      sort_options_box->Add(opt_descending, wxSizerFlags{1}.Expand().Border(wxALL));
      top_sizer->Add(sort_options_box, wxSizerFlags().Expand().Border(wxALL));
      top_sizer->AddSpacer(default_border);

      // filter options box
      m_filter_options_box = new wxStaticBoxSizer(wxVERTICAL, this, constants::LBL_FILTER_OPTIONS);

      // load images for the checkboxes in our filter tree.
      const auto tr_img_size = FromDIP(wxSize{ 16, 16 });
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
      top_sizer->Add(m_filter_options_box, wxSizerFlags(1).Expand().Border(wxALL));
      top_sizer->AddStretchSpacer(2);
      SetSizer(top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &GridOptionsPanel::onSortSelection, this);
      m_filter_tree->Bind(wxEVT_TREE_ITEM_EXPANDING, &GridOptionsPanel::onTreeFilterExpanding, this);
      m_filter_tree->Bind(wxEVT_LEFT_DOWN, &GridOptionsPanel::onTreeFilterLeftClick, this);
      m_score_spin_ctrl->Bind(wxEVT_SPINCTRLDOUBLE, &GridOptionsPanel::onMinScoreChanged, this);
      opt_ascending->Bind(wxEVT_RADIOBUTTON, &GridOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &GridOptionsPanel::onSortOrderClicked, this);
      instock_filter_ctrl->Bind(wxEVT_CHECKBOX, &GridOptionsPanel::OnInStockChecked, this);
      score_filter_chk->Bind(wxEVT_CHECKBOX, &GridOptionsPanel::onMinScoreFilterChecked, this);     
   }


   void GridOptionsPanel::addPropFilter(wxTreeItemId item)
   {
      if (m_sink.hasTable())
      {
         auto filter = getPropFilterForItem(item);
         if (filter)
         {
            m_sink.getTable()->addPropFilterString(filter->propIndex(), m_filter_tree->GetItemText(item).wx_str());
            m_sink.signal_source(GridTableEvent::Id::Filter);
         }
      }
   }


   void GridOptionsPanel::removePropFilter(wxTreeItemId item)
   {
      if (m_sink.hasTable())
      {
         auto filter = getPropFilterForItem(item);
         if (filter)
         {
            m_sink.getTable()->removePropFilterString(filter->propIndex(), m_filter_tree->GetItemText(item).wx_str());
            m_sink.signal_source(GridTableEvent::Id::Filter);
         }
      }
   }


   void GridOptionsPanel::populateFilterTypes(GridTable* grid_table)
   {
      assert(m_filter_tree);

      // disable window updates till we're done and reset the tree
      wxWindowUpdateLocker freeze_updates{m_filter_tree};
      m_filter_tree->DeleteAllItems();
      m_filters.clear();
      m_check_map.clear();

      // get the available filters for this grid table, and add them to the tree.
      auto filters = grid_table->availableStringFilters();
      auto root = m_filter_tree->AddRoot(wxEmptyString);
      for (auto& filter : filters)
      {
         wxString filter_name{ wxFromSV(filter.filterName()) };
         auto item = m_filter_tree->AppendItem(root, filter_name);
         m_filter_tree->SetItemHasChildren(item, true);
         m_filter_tree->SetItemImage(item, IMG_CONTAINER);
         m_filters[item] = filter;
      }
   }


   GridOptionsPanel::MaybeFilter GridOptionsPanel::getPropFilterForItem(wxTreeItemId item)
   {
      auto table = m_sink.getTable();
      if (table)
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


   wxArrayString GridOptionsPanel::getSortOptionList(GridTable* grid_table)
   {
      return vws::all(grid_table->availableSortConfigs()) 
               | vws::transform([](const GridTableSortConfig& s) {  return wxString{s.sort_name.data(), s.sort_name.length() };  })
               | rng::to<wxArrayString>();
   }


   bool GridOptionsPanel::isChecked(wxTreeItemId item)
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CHECKED;
   }


   bool GridOptionsPanel::isContainerNode(wxTreeItemId item)
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CONTAINER;
   }


   bool GridOptionsPanel::isMatchValueNode(wxTreeItemId item)
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) != IMG_CONTAINER;
   }

   /// @brief updates the checked/unchecked status of a node.
   /// @return true if successful, false otherwise (ie invalid item)
   /// 
   bool GridOptionsPanel::setMatchValueChecked(wxTreeItemId item, bool checked)
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
   void GridOptionsPanel::toggleFilterSelection(wxTreeItemId item)
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


   void GridOptionsPanel::updateFilterLabel(wxTreeItemId item)
   {
      auto maybe_filter = m_filters[item];
      if ( !maybe_filter or !item.IsOk() )
         return;

      // if the filter node has selected childen, update the lable with the count
      wxString filter_name{ wxFromSV(maybe_filter->filterName()) };
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


   void GridOptionsPanel::enableInStockFilter(bool enable)
   {
      constexpr size_t index = 0;
      m_filter_options_box->Show(index, enable);
   }


   void GridOptionsPanel::resetInStockCheckbox()
   {
      m_instock_only = false;
      TransferDataToWindow();
   }


   void GridOptionsPanel::notify(GridTableEvent event)
   {
      assert(event.m_grid_table);

      try
      {
         switch (event.m_event_id)
         {
         case GridTableEvent::Id::TableInitialize:
            onTableInitialize(event.m_grid_table);
            enableInStockFilter(event.m_grid_table->hasInStockFilter());
            resetInStockCheckbox();
            break;

         case GridTableEvent::Id::Sort:
            onTableSorted(event.m_grid_table);
            break;

         case GridTableEvent::Id::Filter:
         case GridTableEvent::Id::SubStringFilter:
         case GridTableEvent::Id::RowSelected:
         default:
            break;
         }
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }
   }


   void GridOptionsPanel::onTableInitialize(GridTable* grid_table)
   {
      // reload sort/filter options
      m_sort_combo->Clear();
      m_sort_combo->Append(getSortOptionList(grid_table));
      onTableSorted(grid_table);
      populateFilterTypes(grid_table);
   }


   void GridOptionsPanel::onTableSorted(GridTable* grid_table)
   {
      m_sort_config = grid_table->activeSortConfig();
      TransferDataToWindow();
   }


   void GridOptionsPanel::OnInStockChecked([[maybe_unused]] wxCommandEvent& event)
   {
      assert(m_sink.hasTable());

      TransferDataFromWindow();
      if (m_sink.hasTable() and m_sink.getTable()->enableInStockFilter(m_instock_only))
      {
         m_sink.signal_source(GridTableEvent::Id::Filter);
      }
      else{
         // something went wrong, so clear checkbox.
         m_instock_only = false;
         TransferDataToWindow();
      }
   }


   void GridOptionsPanel::onMinScoreChanged([[maybe_unused]] wxSpinDoubleEvent& event)
   {
      if (m_enable_score_filter and m_sink.hasTable())
      {
         if ( m_sink.getTable()->setMinScoreFilter(event.GetValue()) )
         {
            m_sink.signal_source(GridTableEvent::Id::Filter);
         }
      }
   }


   void GridOptionsPanel::onMinScoreFilterChecked([[maybe_unused]] wxCommandEvent& event)
   {
      if (!m_sink.hasTable())
      {
         assert(false);
         return;
      }

      TransferDataFromWindow();
      m_score_spin_ctrl->Enable(m_enable_score_filter);
      if (m_enable_score_filter) 
      {
         if (m_sink.getTable()->setMinScoreFilter(m_score_filter_val))
         {
            m_sink.signal_source(GridTableEvent::Id::Filter);
         }
      }
      else 
      {
         if (m_sink.getTable()->setMinScoreFilter(std::nullopt))
         {
            m_sink.signal_source(GridTableEvent::Id::Filter);
         }
      }
   }


   void GridOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();
         auto table = m_sink.getTable();
         if (table)
         {
            table->applySortConfig(m_sort_config);
            m_sink.signal_source(GridTableEvent::Id::Sort);
         }
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }

   }


   void GridOptionsPanel::onSortSelection([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         // let the combo close its list before we reload the grid
         CallAfter([this](){
            auto table = m_sink.getTable();
            if (table)
            {
               table->applySortConfig(m_sort_config);
               m_sink.signal_source(GridTableEvent::Id::Sort);
            }
            });
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }
   }


   void GridOptionsPanel::onTreeFilterExpanding(wxTreeEvent& event)
   {
      try
      {

         auto filter_node = event.GetItem();
         if (!filter_node.IsOk())
            return;

         // if the node has a filter in our map and it doesn't already have a list of 
         // available filter values as children, we need to populate the child nodes.
         // if not just return.
         if( !m_filters.contains(filter_node.GetID()) || m_filter_tree->GetChildrenCount(filter_node) > 0 )
         {
            return;
         }

         auto grid_table = m_sink.getTable();
         assert(grid_table);

         auto& filter = m_filters[filter_node.m_pItem]; 
         for (auto& match_val : filter->getMatchValues(grid_table.get()) )
         {
            auto item = m_filter_tree->AppendItem(filter_node, match_val.c_str());
            m_filter_tree->SetItemImage(item, 1);
         }
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }
   }


   void GridOptionsPanel::onTreeFilterLeftClick(wxMouseEvent& event)
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
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }
   }


} // namespace ctb::app