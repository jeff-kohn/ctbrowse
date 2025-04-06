#pragma once

#include "App.h"
#include "grid/ScopedEventSink.h"

#include <wx/splitter.h>
#include <memory>


namespace ctb::app
{
   class CellarTrackerGrid;   // the grid window
   class GridOptionsPanel;    // the options panel
   class WineDetailsPanel;    // details panel

   class LabelImageCache;
   using LabelCachePtr = std::shared_ptr<LabelImageCache>;

   struct IGridTableEventSource;
   using EventSourcePtr = std::shared_ptr<IGridTableEventSource>;


   /// @brief Window class that implements three nested grid views using splitter windows.
   ///
   class GridMultiView final : public wxSplitterWindow, public IGridTableEventSink
   {
   public:
      /// @brief static factory method to create an initialize an instance of the GridPanelsView class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (wx windows are self-deleting).
      /// 
      [[nodiscard]] static auto create(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache) -> GridMultiView*;

      /// @brief Retrieves the grid window for the view.
      /// @return pointer to the CellarTrackerGrid window for this view. will always be valid, not nullptr
      /// 
      template<typename Self>
      auto* grid(this Self&& self)
      {
         return std::forward<Self>(self).m_grid;
      }

      // no copy/move/assign, this class is created on the heap.
      GridMultiView(const GridMultiView&) = delete;
      GridMultiView(GridMultiView&&) = delete;
      GridMultiView& operator=(const GridMultiView&) = delete;
      GridMultiView& operator=(GridMultiView&&) = delete;
      ~GridMultiView() override = default;

   private:
      // non-owning child window pointers.
      wxSplitterWindow*  m_right_splitter{};
      GridOptionsPanel*  m_options_panel{};
      WineDetailsPanel*  m_details_panel{};
      CellarTrackerGrid* m_grid{};
      ScopedEventSink    m_sink;

      GridMultiView(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache);

      // Inherited via IGridTableEventSink
      void notify(GridTableEvent event) override;
   };


} // namespace ctb::app