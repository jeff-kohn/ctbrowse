#include "MultiValueFilterTree.h"

#include <ctb/interfaces/IDatasetEventSink.h>
#include <ctb/interfaces/IDatasetEventSource.h>

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/wupdlock.h>


namespace ctb::app
{
   static constexpr auto WINDOW_STYLE  = wxTR_DEFAULT_STYLE | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS | wxTR_NO_LINES | wxTR_HIDE_ROOT | wxTR_SINGLE;
   static constexpr int  IMG_CONTAINER = 0;
   static constexpr int  IMG_UNCHECKED = 1;
   static constexpr int  IMG_CHECKED   = 2;


   static constexpr auto getPropertyForFieldType(const CtFieldSchema& fld, std::string_view text_val) -> CtPropertyVal
   {
      switch (fld.prop_type)
      {
      case PropType::String:
         return CtPropertyVal{ std::string{ text_val } };

      case PropType::UInt16:
         return CtPropertyVal::parse<uint16_t>(text_val);

      case PropType::UInt64:
         return CtPropertyVal::parse<uint64_t>(text_val);

      case PropType::Double:
         return CtPropertyVal::parse<double>(text_val);

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


   auto MultiValueFilterTree::create(wxWindow& parent, DatasetEventSourcePtr source) -> MultiValueFilterTree*
   {
      return new MultiValueFilterTree{ parent, source };
   }


   MultiValueFilterTree::MultiValueFilterTree(wxWindow& parent, DatasetEventSourcePtr source) :
      wxTreeCtrl{ &parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, WINDOW_STYLE },
      m_sink{ this, source }
   {
      using namespace ctb::constants;

      // load images for the items in our filter tree.
      const auto tr_img_size = wxSize{ 16, 16 };
      m_images.emplace_back(wxBitmapBundle::FromSVGResource(RES_NAME_TREE_FILTER_IMG, tr_img_size));
      m_images.emplace_back(wxBitmapBundle::FromSVGResource(RES_NAME_TREE_UNCHECKED_IMG, tr_img_size));
      m_images.emplace_back(wxBitmapBundle::FromSVGResource(RES_NAME_TREE_CHECKED_IMG, tr_img_size));
      SetImages(m_images);

      Bind(wxEVT_TREE_ITEM_EXPANDING, &MultiValueFilterTree::onNodeExpanding, this);
      Bind(wxEVT_LEFT_DOWN,           &MultiValueFilterTree::onNodeLeftClick, this);
      Bind(wxEVT_TREE_ITEM_MENU,      &MultiValueFilterTree::onNodePopupMenu, this);
      Bind(wxEVT_CONTEXT_MENU,        &MultiValueFilterTree::onTreePopupMenu, this);

      Bind(wxEVT_MENU, &MultiValueFilterTree::onCollapseExpandNode, this, CMD_FILTER_TREE_COLLAPSE_EXPAND);
      Bind(wxEVT_MENU, &MultiValueFilterTree::onCopyValue,          this, wxID_COPY);
      Bind(wxEVT_MENU, &MultiValueFilterTree::onCollapseAllNodes,   this, CMD_FILTER_TREE_COLLAPSE_ALL);
      Bind(wxEVT_MENU, &MultiValueFilterTree::onClearAllFilters,    this, CMD_FILTER_TREE_CLEAR_ALL);
      Bind(wxEVT_MENU, &MultiValueFilterTree::onDeselectAll,        this, CMD_FILTER_TREE_DESELECT_ALL);
      Bind(wxEVT_MENU, &MultiValueFilterTree::onInvertSelection,    this, CND_FILTER_TREE_INVERT_SELECTION);
      Bind(wxEVT_MENU, &MultiValueFilterTree::onToggleChecked,      this, CMD_FILTER_TREE_TOGGLE_CHECKED);

      Bind(wxEVT_UPDATE_UI, &MultiValueFilterTree::onCollapseAllNodesUpdateUI, this, CMD_FILTER_TREE_COLLAPSE_ALL);
      Bind(wxEVT_UPDATE_UI, &MultiValueFilterTree::onClearAllFiltersUpdateUI,  this, CMD_FILTER_TREE_CLEAR_ALL);
      Bind(wxEVT_UPDATE_UI, &MultiValueFilterTree::onDeselectAllUpdateUI,      this, CMD_FILTER_TREE_DESELECT_ALL);
      Bind(wxEVT_UPDATE_UI, &MultiValueFilterTree::onDeselectAllUpdateUI,      this, CND_FILTER_TREE_INVERT_SELECTION); // Same logic as Deselect All

   }


   /// @brief Event dispatcher for IDatasetEventSink
   void MultiValueFilterTree::notify(DatasetEvent event)
   {
      try
      {
         switch (event.event_id)
         {
            case DatasetEvent::Id::Filter:              [[fallthrough]];
            case DatasetEvent::Id::DatasetInitialize:
               onDatasetInitialize(*event.dataset.get());
               break;

            case DatasetEvent::Id::Sort:                [[fallthrough]];
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


   /// @brief Event handler called when a new dataset is associated with the event source
   void MultiValueFilterTree::onDatasetInitialize(IDataset& dataset)
   {
      wxBusyCursor busy{};
      wxWindowUpdateLocker freeze_updates{ this };

      // use available filters to build the top-level tree structure which has a node for each available filter.
      populateFilterNodes(dataset);

      // for active filters, populate their match values, checking as appropriate. the rest will get populated when user expands them.
      for (const auto& [key, filter] : dataset.multivalFilters().activeFilters())
      {
         if (m_name_nodes.contains(filter.filter_name))
         {
            // assign the active filter to our map (so we pick up the selected match values) before populating node children
            auto& filter_node = m_name_nodes[filter.filter_name];
            m_node_filters[filter_node] = filter;
            populateFilterChildItems(filter_node);
         }
         else {
            assert(false and "filter_name should always be in m_name_nodes, this is a bug");
         }
      }
   }


   void MultiValueFilterTree::onCollapseExpandNode([[maybe_unused]] wxCommandEvent& event)
   {
      try 
      {
         auto item = GetSelection();
         if (IsExpanded(item))
         {
            Collapse(item);
         }
         else
         {
            Expand(item);
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onCollapseAllNodes([[maybe_unused]]wxCommandEvent& event)
   {
      try 
      {
         CollapseAll();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onCopyValue([[maybe_unused]] wxCommandEvent& event)
   {
      try 
      {
         if (wxTheClipboard->Open())
         {
            // ownership of data object is transferred to the clipboard
            wxTheClipboard->SetData(new wxTextDataObject{ GetItemText(GetSelection()) });
            wxTheClipboard->Close();
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onClearAllFilters([[maybe_unused]] wxCommandEvent& event)
   {
      try 
      {
         auto dataset = m_sink.getDatasetOrThrow();
         dataset->multivalFilters().clear();
         m_sink.signal_source(DatasetEvent::Id::Filter, true);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onDeselectAll([[maybe_unused]] wxCommandEvent& event)
   {
      try 
      {
         auto item = GetSelection();
         auto current_filter = getFilter(item);
         auto dataset = m_sink.getDatasetOrThrow();
         dataset->multivalFilters().removeFilter(current_filter.prop_id);
         m_sink.signal_source(DatasetEvent::Id::Filter, true);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onInvertSelection([[maybe_unused]] wxCommandEvent& event)
   {
      try 
      {
         auto item = GetSelection();
         auto current_filter = getFilter(item);
         auto dataset = m_sink.getDatasetOrThrow();

         // we want all select all unselected values and vice-versa. so remove current selected values from
         // full list of values to get the list we need to select. 
         CtMultiValueFilter::MatchValues new_values{};
         auto all_values = dataset->getDistinctValues(current_filter.prop_id, false);
         std::ranges::set_difference(all_values, current_filter.match_values, std::inserter(new_values, new_values.begin()));
         current_filter.match_values.swap(new_values);

         dataset->multivalFilters().replaceFilter(current_filter.prop_id, current_filter);
         m_sink.signal_source(DatasetEvent::Id::Filter, true);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onToggleChecked([[maybe_unused]] wxCommandEvent& event)
   {
      try 
      {
         auto item = GetSelection();
         toggleFilterSelection(item);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   /// @brief Event handler fired when user is expanding a filter node to view its match values
   void MultiValueFilterTree::onNodeExpanding(wxTreeEvent& event)
   {
      try
      {
         auto filter_node = event.GetItem();
         if (!isItemFilterNode(filter_node))
         {
            assert("Something got corrupted, should never get invalid node item here" and false);
            throw Error{ constants::ERROR_STR_UNKNOWN, Error::Category::GenericError };
         }

         // if the node already has a list of available filter values as children, we need to clear and repopulate it 
         // because the match values could have changed
         populateFilterChildItems(filter_node);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onNodePopupMenu(wxTreeEvent& event)
   {
      try 
      {
         auto item = event.GetItem();
         if (!item.IsOk())
            return;

         auto menu = getPopupMenu(item);
         SelectItem(item);    // behavior is unintuitive to use without this
         PopupMenu(menu.get(), event.GetPoint());
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MultiValueFilterTree::onTreePopupMenu([[maybe_unused]] wxContextMenuEvent& event)
   {
      try 
      {
         auto menu = getPopupMenu(nullptr);
         PopupMenu(menu.get());
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   /// @brief Event handler fired when user left-clicks on a tree item.
   void MultiValueFilterTree::onNodeLeftClick(wxMouseEvent& event)
   {
      try
      {
         int flags{};
         auto item = HitTest(event.GetPosition(), flags);

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


   void MultiValueFilterTree::onCollapseAllNodesUpdateUI(wxUpdateUIEvent& event)
   {
      event.Enable(rng::find_if(m_node_filters, [this](const auto& map_elem) { return IsExpanded(map_elem.first); }) != m_node_filters.end());
   }


   void MultiValueFilterTree::onClearAllFiltersUpdateUI(wxUpdateUIEvent& event)
   {
      event.Enable(rng::find_if(m_check_counts, [](const auto& map_elem) { return map_elem.second > 0; }) != m_check_counts.end());
   }


   void MultiValueFilterTree::onDeselectAllUpdateUI(wxUpdateUIEvent& event)
   {
      // Does filter node have at least one value checked?
      event.Enable(rng::find_if(m_node_filters, [](const auto& map_elem) { return map_elem.second.match_values.size() > 0; }) != m_node_filters.end());
   }

   /// @return A reference to the filter object associated with the specified tree item. 
   /// 
   /// Note this filter is from our internal map, not dataset's activeMultivalFilters()
   auto MultiValueFilterTree::getFilter(wxTreeItemId item)  noexcept(false) -> CtMultiValueFilter&
   {
      auto parent_node = GetItemParent(item);
      auto filter_node = parent_node == GetRootItem() ? item : parent_node;
   
      if (filter_node.IsOk())
      {
         auto it = m_node_filters.find(filter_node);
         if (it != m_node_filters.end())
         {
            return it->second;
         }
      }
      throw Error{ constants::ERROR_STR_FILTER_NOT_FOUND, Error::Category::DataError };
   }


   /// @return an appropriately-typed CtPropertyVal representing the specified tree item's value
   auto MultiValueFilterTree::getFilterValue(wxTreeItemId item) -> CtPropertyVal
   {
      auto dataset    = m_sink.getDatasetOrThrow();
      auto& filter    = getFilter(item);
      auto fld_schema = dataset->getFieldSchema(filter.prop_id);

      if (fld_schema.has_value())
      {
         /// We need to convert the string value from the filter node's label to the correct type, which may not be string.
         return getPropertyForFieldType(*fld_schema, wxViewString(GetItemText(item)));
      }
      else {
         assert("Not getting a valid FieldSchema here is a bug." and false);
         return { GetItemText(item).utf8_string() };
      }
   }


   auto MultiValueFilterTree::getPopupMenu(wxTreeItemId item) const -> wxMenuPtr
   {
      using namespace constants;

      auto popup_menu = std::make_unique<wxMenu>();

      if (isItemMatchValueNode(item))
      {
         // Copy to clipboard
         auto* menu_copy = new wxMenuItem{ popup_menu.get(), wxID_COPY};
         menu_copy->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_COPY, wxART_MENU));
         popup_menu->Append(menu_copy);
         popup_menu->AppendSeparator();

         // Check/Uncheck filter
         bool is_checked = isItemChecked(item);
         auto lbl = is_checked ? CMD_FILTER_TREE_UNCHECK_FILTER_LBL : CMD_FILTER_TREE_CHECK_FILTER_LBL;
         auto tip = is_checked ? CMD_FILTER_TREE_UNCHECK_FILTER_TIP : CMD_FILTER_TREE_CHECK_FILTER_TIP;
         popup_menu->Append(new wxMenuItem{ popup_menu.get(), CmdId::CMD_FILTER_TREE_TOGGLE_CHECKED, lbl, tip, wxITEM_NORMAL });
      }
      else if (isItemFilterNode(item))
      {
         // Collapse/Expand
         bool is_expanded = IsExpanded(item);
         auto lbl = is_expanded ? CMD_FILTER_TREE_COLLAPSE_LBL : CMD_FILTER_TREE_EXPAND_LBL;
         auto tip = is_expanded ? CMD_FILTER_TREE_COLLAPSE_TIP : CMD_FILTER_TREE_EXPAND_TIP;
         popup_menu->Append(new wxMenuItem{ popup_menu.get(), CmdId::CMD_FILTER_TREE_COLLAPSE_EXPAND, lbl, tip, wxITEM_NORMAL });
         popup_menu->AppendSeparator();

         // Deselect all
         lbl = CMD_FILTER_TREE_DESELECT_ALL_LBL;
         tip = CMD_FILTER_TREE_DESELECT_ALL_TIP;
         popup_menu->Append(new wxMenuItem{ popup_menu.get(), CmdId::CMD_FILTER_TREE_DESELECT_ALL, lbl, tip, wxITEM_NORMAL });

         // Invert Selectioin
         lbl = CMD_FILTER_TREE_INVERT_LBL;
         tip = CMD_FILTER_TREE_INVERT_TIP;
         popup_menu->Append(new wxMenuItem{ popup_menu.get(), CmdId::CND_FILTER_TREE_INVERT_SELECTION, lbl, tip, wxITEM_NORMAL });
      }
      else {
         // collapse all
         auto lbl = CMD_FILTER_TREE_COLLAPSE_ALL_LBL;
         auto tip = CMD_FILTER_TREE_COLLAPSE_ALL_TIP;
         popup_menu->Append(new wxMenuItem{ popup_menu.get(), CmdId::CMD_FILTER_TREE_COLLAPSE_ALL, lbl, tip, wxITEM_NORMAL });
         popup_menu->AppendSeparator();

         // clear 
         lbl = CMD_FILTER_TREE_CLEAR_ALL_LBL;
         tip = CMD_FILTER_TREE_CLEAR_ALL_TIP;
         popup_menu->Append(new wxMenuItem{ popup_menu.get(), CmdId::CMD_FILTER_TREE_CLEAR_ALL, lbl, tip, wxITEM_NORMAL });
      }

      return popup_menu;
   }


   /// @return true if the specified tree item is a checked match-value 
   auto MultiValueFilterTree::isItemChecked(wxTreeItemId item) const -> bool
   {
      return item.IsOk() and GetItemImage(item) == IMG_CHECKED;
   }


   /// @return true if item represents a filter node containing match values as children
   auto MultiValueFilterTree::isItemFilterNode(wxTreeItemId item) const -> bool
   {
      return item.IsOk() and m_node_filters.contains(item.GetID());
   }


   /// @return true if the item represents a match value for a filter, false otherwise
   auto MultiValueFilterTree::isItemMatchValueNode(wxTreeItemId item) const -> bool
   {
      return item.IsOk() and GetItemImage(item) != IMG_CONTAINER;
   }


   /// @brief Add the tree-item's value as a match value for it's parent filter.
   /// 
   /// This function does not update the UI to reflect the newly added filter value,
   /// call SetCheck() for that.
   void MultiValueFilterTree::enableFilterMatchValue(wxTreeItemId item) noexcept(false)
   {
      auto dataset = m_sink.getDatasetOrThrow();
      auto& filter = getFilter(item);

      auto&& [iter, was_inserted] = filter.match_values.insert(getFilterValue(item));
      if (was_inserted)
      {
         dataset->multivalFilters().replaceFilter(filter.prop_id, filter);
         m_sink.signal_source(DatasetEvent::Id::Filter, false); 
      }
   }


   /// @brief Reinitializes the tree-view with a list of top-level nodes representing the available filters for the dataset.
   /// 
   /// Child nodes are not populated, that happens in onNodeExpanding()
   /// 
   void MultiValueFilterTree::populateFilterNodes(IDataset& dataset)
   {
      m_check_counts.clear();
      m_name_nodes.clear();
      m_node_filters.clear();
      DeleteAllItems();

      // get the available filters for this dataset, and add them to the tree.
      auto filters = dataset.availableMultiValueFilters();
      auto root = AddRoot(wxEmptyString);
      for (const auto& filter : filters)
      {
         auto filter_node = AppendItem(root, filter.filter_name);
         SetItemHasChildren(filter_node, true);
         SetItemImage(filter_node, IMG_CONTAINER);
         m_node_filters[filter_node] = filter;
         m_name_nodes[filter.filter_name] = filter_node;
      }
   }


   void MultiValueFilterTree::populateFilterChildItems(wxTreeItemId filter_node) noexcept(false)
   {
      auto current_filter = getFilter(filter_node);
      auto dataset = m_sink.getDatasetOrThrow();

      DeleteChildren(filter_node);
      clearCheckCounts(filter_node);

      /// build a copy of dataset's filter-mgr that has all multi-val filters enabled except for the one we're getting values for
      /// (if it was enabled, we wouldn't get any match values besides those already selected)
      CtMultiValueFilterMgr custom_filters{};
      for (const auto& filter : vws::values(dataset->multivalFilters().activeFilters()))
      {
         if (filter.prop_id != current_filter.prop_id)
         {
            custom_filters.replaceFilter(filter.prop_id, filter);
         }
      }

      auto createChildNode = [&current_filter, &filter_node, this](const CtPropertyVal& match_value)
         {
            auto match_str = match_value.asString();
            auto item = AppendItem(filter_node, match_str);
            SetItemImage(item, IMG_UNCHECKED);
            if (current_filter.match_values.contains(match_value))
            {
               setChecked(item, true);
            }
         };

      // Check which order the filter values should be sorted, some are descending
      if (current_filter.reverse_match_values)
      {
         rng::for_each(vws::reverse(dataset->getDistinctValues(current_filter.prop_id, custom_filters)), createChildNode);
      }
      else {
         rng::for_each(dataset->getDistinctValues(current_filter.prop_id, custom_filters), createChildNode);
      }
      
      updateFilterLabel(filter_node);
   }


   void MultiValueFilterTree::clearCheckCounts(wxTreeItemId filter_node)
   {
      if (isItemFilterNode(filter_node))
      {
         for (auto& [id, count] : m_check_counts)
         {
            if (id == filter_node)
               count = 0;
         }
      }
   }


   /// @brief Removes the match value represented by the item from the filter's active match values.
   void MultiValueFilterTree::removeFilter(wxTreeItemId item) noexcept(false)
   {
      auto& filter = getFilter(item);
      auto dataset = m_sink.getDatasetOrThrow();

      if (filter.match_values.erase(getFilterValue(item)))
      {
         dataset->multivalFilters().replaceFilter(filter.prop_id, filter);
         m_sink.signal_source(DatasetEvent::Id::Filter, false); 
      }
   }


   /// @brief updates the checked/unchecked status of a node.
   void MultiValueFilterTree::setChecked(wxTreeItemId item, bool checked)
   {
      if (!isItemMatchValueNode(item))
         return;

      // update the image to checkmark and update info for filter count in tree label 
      if (checked)
      {
         SetItemImage(item, IMG_CHECKED);
         m_check_counts[GetItemParent(item)]++;
      }
      else{
         SetItemImage(item, IMG_UNCHECKED);
         m_check_counts[GetItemParent(item)]--;
      }
      updateFilterLabel(GetItemParent(item));
   }


   /// @brief toggles a filter value by updating its checked/unchecked image and 
   ///  applying/deleting the corresponding filter.
   void MultiValueFilterTree::toggleFilterSelection(wxTreeItemId item)
   {
      if (!isItemMatchValueNode(item))
         return;

      if (isItemChecked(item))
      {
         removeFilter(item);
         setChecked(item, false);
      }
      else{ 
         enableFilterMatchValue(item);
         setChecked(item, true);
      }
   }


   /// @brief Updates the label for a filter node to show the number of enabled match value it contains
   void MultiValueFilterTree::updateFilterLabel(wxTreeItemId item)
   {
      if (!isItemFilterNode(item))
         return;

      auto& filter = m_node_filters[item];
      auto count   = m_check_counts[item];
      auto lbl     = count ? ctb::format(constants::FMT_LBL_FILTERS_SELECTED, filter.filter_name, count) : filter.filter_name;
      SetItemText(item, lbl);
   }

} // namespace ctb::app
