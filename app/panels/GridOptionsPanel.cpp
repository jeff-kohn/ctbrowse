/*******************************************************************
 * @file GridOptionsPanel.cpp
 *
 * @brief implementation file for the GridOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "App.h"
#include "wx_helpers.h"
#include "panels/GridOptionsPanel.h"

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
   }
   
   GridOptionsPanel::GridOptionsPanel(GridTableEventSourcePtr source) : m_sink{ this, source }
   {}


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
      //SetMaxSize(ConvertDialogToPixels(wxSize{ 140, constants::WX_UNSPECIFIED_VALUE }));
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
      auto* filter_options_box = new wxStaticBoxSizer(wxVERTICAL, this, constants::LBL_FILTER_OPTIONS);

      // load images for the checkboxes in our filter tree.
      const auto tr_img_size = FromDIP(wxSize{ 16, 16 });
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(constants::RES_NAME_TREE_FILTER_IMG, tr_img_size));
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(constants::RES_NAME_TREE_UNCHECKED_IMG, tr_img_size));
      m_filter_tree_images.emplace_back(wxBitmapBundle::FromSVGResource(constants::RES_NAME_TREE_CHECKED_IMG, tr_img_size));

      // filter tree control
      auto style = wxTR_DEFAULT_STYLE | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT;
      m_filter_tree = new wxTreeCtrl{ filter_options_box->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, style };
      m_filter_tree->SetMaxSize(ConvertDialogToPixels(wxSize(-1, 500)));
      m_filter_tree->SetMinSize(ConvertDialogToPixels(wxSize(-1, 100)));
      m_filter_tree->SetImages(m_filter_tree_images);
      filter_options_box->Add(m_filter_tree, wxSizerFlags(2).Expand().Border(wxALL));
      filter_options_box->AddSpacer(default_border);

      // finalize layout
      top_sizer->Add(filter_options_box, wxSizerFlags(1).Expand().Border(wxALL));
      top_sizer->AddStretchSpacer(2);
      SetSizer(top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &GridOptionsPanel::onSortSelection, this);
      m_filter_tree->Bind(wxEVT_TREE_ITEM_EXPANDING, &GridOptionsPanel::onTreeFilterExpanding, this);
      m_filter_tree->Bind(wxEVT_LEFT_DOWN, &GridOptionsPanel::onTreeFilterLeftClick, this);
      opt_ascending->Bind(wxEVT_RADIOBUTTON, &GridOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &GridOptionsPanel::onSortOrderClicked, this);

   }


   void GridOptionsPanel::addFilter(wxTreeItemId item)
   {
      if (m_sink.hasTable())
      {
         auto filter = getFilterForItem(item);
         if (filter)
         {
            m_sink.getTable()->addFilter(filter->propIndex(), m_filter_tree->GetItemText(item).wx_str());
            m_sink.signal_source(GridTableEvent::Filter);
         }
      }
   }


   void GridOptionsPanel::removeFilter(wxTreeItemId item)
   {
      if (m_sink.hasTable())
      {
         auto filter = getFilterForItem(item);
         if (filter)
         {
            m_sink.getTable()->removeFilter(filter->propIndex(), m_filter_tree->GetItemText(item).wx_str());
            m_sink.signal_source(GridTableEvent::Filter);
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
      auto filters = grid_table->availableFilters();
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


   wxArrayString GridOptionsPanel::getSortOptionList(GridTable* grid_table)
   {
      return vws::all(grid_table->availableSortConfigs()) 
               | vws::transform([](const GridTableSortConfig& s) {  return wxString{s.sort_name.data(), s.sort_name.length() };  })
               | rng::to<wxArrayString>();
   }


   GridOptionsPanel::MaybeFilter GridOptionsPanel::getFilterForItem(wxTreeItemId item)
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


   bool GridOptionsPanel::isContainerNode(wxTreeItemId item)
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CONTAINER;
   }


   bool GridOptionsPanel::isMatchValueNode(wxTreeItemId item)
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) != IMG_CONTAINER;
   }


   bool GridOptionsPanel::isChecked(wxTreeItemId item)
   {
      return item.IsOk() and m_filter_tree->GetItemImage(item) == IMG_CHECKED;
   }


   /// @brief updates the checked/unchecked status of a node.
   /// @return true if successful, false otherwise (ie invalid item)
   /// 
   bool GridOptionsPanel::setChecked(wxTreeItemId item, bool checked)
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
         auto lbl = std::format(constants::FMT_LBL_FILTERS_SELECTED, filter_name.wx_str(), count);
         m_filter_tree->SetItemText(item, lbl.c_str());
      }
      else{
         m_filter_tree->SetItemText(item, filter_name);
      }
   }


   /// @brief toggles a filter value by updating its checked/unchecked image and applying/deleting the corresponding filter.
   /// 
   void GridOptionsPanel::toggleFilterSelection(wxTreeItemId item)
   {
      bool checked = !isChecked(item);
      if (!setChecked(item, checked))
         return;

      if (checked)
      {
         addFilter(item);
         m_check_map[item]++;
      }
      else{
         removeFilter(item);
         m_check_map[item]--;
      }
      updateFilterLabel(m_filter_tree->GetItemParent(item));
   }


   void GridOptionsPanel::notify(GridTableEvent event, GridTable* grid_table)
   {
      try
      {
         switch (event)
         {
         case GridTableEvent::TableInitialize:
            onTableInitialize(grid_table);
            break;

         case GridTableEvent::Sort:
            onTableSorted(grid_table);
            break;

         case GridTableEvent::Filter:
         case GridTableEvent::SubStringFilter:
         case GridTableEvent::RowSelected:
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
               m_sink.signal_source(GridTableEvent::Sort);
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


   void GridOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();
         auto table = m_sink.getTable();
         if (table)
         {
            table->applySortConfig(m_sort_config);
            m_sink.signal_source(GridTableEvent::Sort);
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