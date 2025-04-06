#include "views/GridMultiView.h"
#include "views/CellarTrackerGrid.h"
#include "views/GridOptionsPanel.h"
#include "views/WineDetailsPanel.h"

#include <wx/persist/splitter.h>


namespace ctb::app
{
   auto GridMultiView::create(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache) -> GridMultiView*
   {
      try
      {
         std::unique_ptr<GridMultiView> wnd{ new GridMultiView{ parent, source, cache } };
         return wnd.release(); // top-level window manages its own lifetime, we return non-owning pointer
      }
      catch (...)
      {
         auto e = packageError();
         log::exception(e);
         throw e;
      }
   }


   GridMultiView::GridMultiView(wxWindow* parent, EventSourcePtr source, LabelCachePtr cache) : wxSplitterWindow{ parent }
   {
      // first create the nested splitter window, contains the grid and detail views
      m_right_splitter = new wxSplitterWindow{ this };
      m_grid = CellarTrackerGrid::create(m_right_splitter, source);
      m_details_panel = WineDetailsPanel::create(m_right_splitter, source, cache);
      m_right_splitter->SplitVertically(m_grid, m_details_panel);

      // Now create the options panel, and add it to "this" splitter along with the nested one
      m_options_panel = GridOptionsPanel::create(this, source);
      SplitVertically(m_options_panel, m_right_splitter);

      //wxPersistentRegisterAndRestore(m_right_splitter, "right_splitter");
      //wxPersistentRegisterAndRestore(this, "left_splitter");

      // Need to set these after restoring window states, or it messes things up.
      m_right_splitter->SetSashGravity(1);
      SetSashGravity(0.1);
   }


} // namespace ctb::app