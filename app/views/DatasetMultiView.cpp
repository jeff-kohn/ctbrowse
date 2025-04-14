#include "views/DatasetListView.h"
#include "views/DatasetMultiView.h"
#include "views/DatasetOptionsPanel.h"
#include "views/DetailsPanel.h"

#include <wx/persist/splitter.h>


namespace ctb::app
{
   auto DatasetMultiView::create(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache) -> DatasetMultiView*
   {
      try
      {
         std::unique_ptr<DatasetMultiView> wnd{ new DatasetMultiView{ parent, source, cache } };
         return wnd.release(); // top-level window manages its own lifetime, we return non-owning pointer
      }
      catch (...)
      {
         auto e = packageError();
         log::exception(e);
         throw e;
      }
   }


   DatasetMultiView::DatasetMultiView(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache) : 
      wxSplitterWindow{ parent },
      m_sink{ this, source }
   {
      constexpr auto LEFT_SPLITTER_GRAVITY = 0.25;
      constexpr auto RIGHT_SPLITTER_GRAVITY = 0.75;

      SetName("DatasetMultiView");
      SetSashGravity(LEFT_SPLITTER_GRAVITY);

      // top/left splitter contains options panel and right/nested splitter
      m_right_splitter = new wxSplitterWindow{ this };
      m_options_panel = DatasetOptionsPanel::create(this, source);
      SplitVertically(m_options_panel, m_right_splitter);
      wxPersistentRegisterAndRestore(this, GetName());

      // nested splitter contains grid and details1
      m_grid = DatasetListView::create(m_right_splitter, source);
      m_details_panel = DetailsPanel::create(m_right_splitter, source, cache);
      m_right_splitter->SplitVertically(m_grid, m_details_panel);
      m_right_splitter->SetName("GridMultiViewNested");
      wxPersistentRegisterAndRestore(m_right_splitter, m_right_splitter->GetName());
      
      // For some reason the CalLAfter() is required, otherwise this call messes up the
      // next splitter layout. No idea why but this works.
      CallAfter([this] { m_right_splitter->SetSashGravity(RIGHT_SPLITTER_GRAVITY); });
   }

   void DatasetMultiView::notify(DatasetEvent event)
   {
      switch (event.m_event_id){
         case DatasetEvent::Id::TableRemove: [[fallthrough]];
         case DatasetEvent::Id::RowSelected: [[fallthrough]];
         case DatasetEvent::Id::GridLayoutRequested:
            break;

         default:
            // make sure everyone has had a chance to handle current event before generating a new one.
            CallAfter([this] {
               if (m_grid)
               {
                  //m_grid->SelectRow(0);
                  m_sink.signal_source(DatasetEvent::Id::RowSelected, 0);
               }
            });
      }
   }


} // namespace ctb::app