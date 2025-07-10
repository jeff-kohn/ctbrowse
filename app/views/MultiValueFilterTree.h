#pragma once

#include "App.h"

#include <ctb/interfaces/IDatasetEventSink.h>
#include <ctb/interfaces/IDatasetEventSource.h>
#include <ctb/model/ScopedEventSink.h>

#include <wx/treectrl.h>

#include <map>


namespace ctb::app
{

   class MultiValueFilterTree final : public wxTreeCtrl, public IDatasetEventSink
   {
   public:

      /// @brief static factory method to create an initialize an instance of the GridPanelsView class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (parent owns/manages lifetime).
      /// 
      [[nodiscard]] static 
      auto create(wxWindow& parent, std::shared_ptr<IDatasetEventSource> source) -> MultiValueFilterTree*;

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
      using CheckCountMap = std::map<wxTreeItemId, int>;                 // used to track number of checked value nodes a given parent/filter node has 

      CheckCountMap         m_check_counts{};   // for keeping track of number of values selected for a filter/node.
      NameNodeMap           m_name_nodes{};
      NodeFilterMap         m_node_filters{}; 
      ScopedEventSink       m_sink;    
      wxWithImages::Images  m_images{};

      MultiValueFilterTree(wxWindow& parent, std::shared_ptr<IDatasetEventSource> source);

      void notify(DatasetEvent event) override;
      void onDatasetInitialize(IDataset& dataset);

      void onNodeExpanding(wxTreeEvent& event);
      void onNodeLeftClick(wxMouseEvent& event);

      auto isItemChecked(wxTreeItemId item) -> bool;
      auto isItemFilterNode(wxTreeItemId item) -> bool;
      auto isItemMatchValueNode(wxTreeItemId item) -> bool;

      void disableFilterMatchValue(wxTreeItemId item) noexcept(false);
      void enableFilterMatchValue(wxTreeItemId item) noexcept(false) ;
      auto getFilter(wxTreeItemId item) noexcept(false) -> CtMultiValueFilter&;
      auto getFilterValue(wxTreeItemId item) -> CtPropertyVal;
      void populateFilterNodes(IDataset& dataset);
      void setChecked(wxTreeItemId item, bool checked = true);
      void toggleFilterSelection(wxTreeItemId item);
      void updateFilterLabel(wxTreeItemId item);
   };


} // namespace ctb::app