#pragma once

#include "App.h"
#include <ctb/interfaces/IDatasetEventSource.h>
#include <wx/splitter.h>
#include <memory>


namespace ctb::app
{
   class DatasetListView;     // the wine-list window
   class DatasetOptionsView;  // the sort/filter options panel
   class DetailsViewBase;     // details panel


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
      auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> DatasetMultiView*;


      // no copy/move/assign, this class is created on the heap.
      DatasetMultiView(const DatasetMultiView&) = delete;
      DatasetMultiView(DatasetMultiView&&) = delete;
      DatasetMultiView& operator=(const DatasetMultiView&) = delete;
      DatasetMultiView& operator=(DatasetMultiView&&) = delete;
      ~DatasetMultiView() override = default;

   private:
      // non-owning child window pointers.
      DatasetOptionsView*  m_options_panel{};
      DetailsViewBase*     m_details_panel{};
      DatasetListView*     m_listView{};
      wxSplitterWindow*    m_right_splitter{};

      using wxSplitterWindow::Create;
      void createWindow(wxWindow* parent, const DatasetEventSourcePtr& event_source);

      /// @brief Private constructor, used by DatasetMultiView::create()
      DatasetMultiView() = default;
   };


} // namespace ctb::app 