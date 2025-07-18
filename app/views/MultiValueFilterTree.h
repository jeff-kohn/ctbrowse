#pragma once

#include "App.h"

#include <ctb/model/ScopedEventSink.h>
#include <wx/menu.h>
#include <wx/treectrl.h>

#include <map>


namespace ctb::app
{
   /// custom TreeView control that displays a list MultiValueFilters and their match values, so that filters
   /// can be configured by checking/unchecking the relevant filter values.
   /// 
   /// This class is an event sink for IDatasetEventSource and automatically handles updates from other views
   /// as well as notifying other views about changes made. 
   class MultiValueFilterTree final : public wxTreeCtrl, public IDatasetEventSink
   {
   public:

      /// @brief static factory method to create an initialize an instance of the GridPanelsView class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (parent owns/manages lifetime).
      /// 
      [[nodiscard]] static 
      auto create(wxWindow& parent, DatasetEventSourcePtr source) -> MultiValueFilterTree*;

      // no copy/move/assign, this class is created on the heap.
      MultiValueFilterTree() = delete;
      MultiValueFilterTree(const MultiValueFilterTree&) = delete;
      MultiValueFilterTree(MultiValueFilterTree&&) = delete;
      MultiValueFilterTree& operator=(const MultiValueFilterTree&) = delete;
      MultiValueFilterTree& operator=(MultiValueFilterTree&&) = delete;
      ~MultiValueFilterTree() override = default;

   private:
      using NodeFilterMap = std::map<wxTreeItemId, CtMultiValueFilter>;  // maps tree node to corresponding filter 
      using NameNodeMap   = std::map<std::string,  wxTreeItemId>;        // map filter name (must be unique) to tree node
      using CheckCountMap = std::map<wxTreeItemId, size_t>;              // used to track number of checked value nodes a given parent/filter node has 
      using wxMenuPtr     = std::unique_ptr<wxMenu>;                     // type alias for a wxMenu smart ptr 

      CheckCountMap         m_check_counts{};   // for keeping track of number of values selected for a filter/node.
      NameNodeMap           m_name_nodes{};
      NodeFilterMap         m_node_filters{}; 
      ScopedEventSink       m_sink;    
      wxWithImages::Images  m_images{};

      MultiValueFilterTree(wxWindow& parent, DatasetEventSourcePtr source);

      void notify(DatasetEvent event) override;
      void onDatasetInitialize(IDataset& dataset);

      void onCollapseExpandNode(wxCommandEvent& event);
      void onCollapseAllNodes(wxCommandEvent& event);
      void onCopyValue(wxCommandEvent& event);
      void onClearAllFilters(wxCommandEvent& event);
      void onDeselectAll(wxCommandEvent& event);
      void onInvertSelection(wxCommandEvent& event);
      void onToggleChecked(wxCommandEvent& event);
      void onNodeExpanding(wxTreeEvent& event);
      void onNodePopupMenu(wxTreeEvent& event);
      void onTreePopupMenu(wxContextMenuEvent& event); // when right-clicking in client area and not a node.
      void onNodeLeftClick(wxMouseEvent& event);

      void onCollapseAllNodesUpdateUI(wxUpdateUIEvent& event);
      void onClearAllFiltersUpdateUI(wxUpdateUIEvent& event);
      void onDeselectAllUpdateUI(wxUpdateUIEvent& event);

      auto isItemChecked(wxTreeItemId item) const -> bool;
      auto isItemFilterNode(wxTreeItemId item) const -> bool;
      auto isItemMatchValueNode(wxTreeItemId item) const -> bool;

      void clearCheckCounts(wxTreeItemId filter_node);
      void removeFilter(wxTreeItemId item) noexcept(false);
      void enableFilterMatchValue(wxTreeItemId item) noexcept(false) ;

      auto getFilter(wxTreeItemId item) noexcept(false) -> CtMultiValueFilter&;
      auto getFilterValue(wxTreeItemId item) -> CtPropertyVal;
      auto getPopupMenu(wxTreeItemId item) const -> wxMenuPtr;
      void populateFilterNodes(IDataset& dataset);
      void populateFilterChildItems(wxTreeItemId filter_node) noexcept(false);
      void setChecked(wxTreeItemId item, bool checked = true);
      void toggleFilterSelection(wxTreeItemId item);
      void updateFilterLabel(wxTreeItemId item);
   };


} // namespace ctb::app