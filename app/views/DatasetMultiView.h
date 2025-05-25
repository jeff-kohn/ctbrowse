#pragma once

#include "App.h"

#include <wx/splitter.h>
#include <memory>


namespace ctb
{
   struct IDatasetEventSource; // no need to pull in a bunch of datamodel headers
}

namespace ctb::app
{
   class DatasetListView;      // the wine-list window
   class DatasetOptionsPanel;  // the sort/filter options panel
   class DetailsPanel;         // details panel

   class LabelImageCache;
   using LabelCachePtr = std::shared_ptr<LabelImageCache>;


   /// @brief Window class that implements three side-by-side views using splitter windows.
   ///
   class DatasetMultiView final : public wxSplitterWindow
   {
   public:
      /// @brief static factory method to create an initialize an instance of the GridPanelsView class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (wx windows are self-deleting).
      /// 
      [[nodiscard]] static 
      auto create(wxWindow* parent, std::shared_ptr<IDatasetEventSource> source, LabelCachePtr cache) -> DatasetMultiView*;


      /// @brief Indicates whether the details for a selected wine are currently displayed.
      /// @return true if a wine is displayed in details, false otherwise.
      /// 
      auto wineDetailsActive() const -> bool;

      // no copy/move/assign, this class is created on the heap.
      DatasetMultiView(const DatasetMultiView&) = delete;
      DatasetMultiView(DatasetMultiView&&) = delete;
      DatasetMultiView& operator=(const DatasetMultiView&) = delete;
      DatasetMultiView& operator=(DatasetMultiView&&) = delete;
      ~DatasetMultiView() override = default;

   private:
      // non-owning child window pointers.
      DatasetOptionsPanel* m_options_panel{};
      DetailsPanel*        m_details_panel{};
      DatasetListView*     m_listView{};
      wxSplitterWindow*    m_right_splitter{};

      /// @brief Private constructor, used by DatasetMultiView::create()
      DatasetMultiView(wxWindow* parent, std::shared_ptr<IDatasetEventSource> source, LabelCachePtr cache);
   };


} // namespace ctb::app