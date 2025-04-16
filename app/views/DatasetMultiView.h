#pragma once

#include "App.h"
#include "model/ScopedEventSink.h"

#include <wx/splitter.h>
#include <memory>


namespace ctb::app
{
   class DatasetListView;      // the wine-list window
   class DatasetOptionsPanel;  // the sort/filter options panel
   class DetailsPanel;         // details panel

   class LabelImageCache;
   using LabelCachePtr = std::shared_ptr<LabelImageCache>;

   struct IDatasetEventSource;
   using EventSourcePtr = std::shared_ptr<IDatasetEventSource>;


   /// @brief Window class that implements three side-by-side views using splitter windows.
   ///
   class DatasetMultiView final : public wxSplitterWindow, public IDatasetEventSink
   {
   public:
      /// @brief static factory method to create an initialize an instance of the GridPanelsView class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (wx windows are self-deleting).
      /// 
      [[nodiscard]] static auto create(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache) -> DatasetMultiView*;

      /// @brief Retrieves pointer to the child listview window.
      /// Will always be valid, not nullptr
      /// 
      template<typename Self>
      auto* listView(this Self&& self)
      {
         return std::forward<Self>(self).m_listView;
      }

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
      ScopedEventSink      m_sink;
      wxSplitterWindow*    m_right_splitter{};

      /// @brief Private constructor, used by DatasetMultiView::create()
      DatasetMultiView(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache);

      // notification of dataset events
      void notify(DatasetEvent event) override;
   };


} // namespace ctb::app