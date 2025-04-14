#pragma once

#include "App.h"
#include "model/ScopedEventSink.h"

#include <wx/splitter.h>
#include <memory>


namespace ctb::app
{
   class CellarTrackerGrid;   // the grid window
   class DatasetOptionsPanel;    // the options panel
   class DetailsPanel;    // details panel

   class LabelImageCache;
   using LabelCachePtr = std::shared_ptr<LabelImageCache>;

   struct IDatasetEventSource;
   using EventSourcePtr = std::shared_ptr<IDatasetEventSource>;


   /// @brief Window class that implements three nested grid views using splitter windows.
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

      /// @brief Retrieves the grid window for the view.
      /// @return pointer to the CellarTrackerGrid window for this view. will always be valid, not nullptr
      /// 
      template<typename Self>
      auto* grid(this Self&& self)
      {
         return std::forward<Self>(self).m_grid;
      }

      // no copy/move/assign, this class is created on the heap.
      DatasetMultiView(const DatasetMultiView&) = delete;
      DatasetMultiView(DatasetMultiView&&) = delete;
      DatasetMultiView& operator=(const DatasetMultiView&) = delete;
      DatasetMultiView& operator=(DatasetMultiView&&) = delete;
      ~DatasetMultiView() override = default;

   private:
      // non-owning child window pointers.
      wxSplitterWindow*  m_right_splitter{};
      DatasetOptionsPanel*  m_options_panel{};
      DetailsPanel*  m_details_panel{};
      wxWindow* m_grid{};
      ScopedEventSink    m_sink;

      DatasetMultiView(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache);

      // Inherited via IDatasetEventSink
      void notify(DatasetEvent event) override;
   };


} // namespace ctb::app