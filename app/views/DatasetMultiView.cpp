#include "views/DatasetListView.h"
#include "views/DatasetMultiView.h"
#include "views/DatasetOptionsPanel.h"
#include "views/DetailsPanel.h"

#include <wx/persist/splitter.h>


namespace ctb::app
{
   auto DatasetMultiView::create(wxWindow& parent, DatasetEventSourcePtr source, LabelCachePtr cache) -> DatasetMultiView*
   {
      try
      {
         if (!source)
         {
            throw Error{ constants::ERROR_STR_NULLPTR_ARG, Error::Category::ArgumentError };
         }
         std::unique_ptr<DatasetMultiView> wnd{ new DatasetMultiView{ &parent, source, cache } };
         return wnd.release(); // top-level window manages its own lifetime, we return non-owning pointer
      }
      catch (...){
         log::exception(packageError());
         throw;
      }
   }

   
   DatasetMultiView::DatasetMultiView(wxWindow* parent, DatasetEventSourcePtr source, LabelCachePtr cache) : wxSplitterWindow{ parent }
   {
      constexpr auto LEFT_SPLITTER_GRAVITY = 0.25;
      constexpr auto RIGHT_SPLITTER_GRAVITY = 0.75;
      constexpr auto MIN_PANE_SIZE = 100;

      auto font = GetFont();
      font.SetPointSize(font.GetPointSize() + 1);
      SetFont(font);

      SetSashGravity(LEFT_SPLITTER_GRAVITY);

      // this splitter window contains options panel and right/nested splitter
      m_options_panel = DatasetOptionsPanel::create(this, source);
      m_right_splitter = new wxSplitterWindow{ this };
      SplitVertically(m_options_panel, m_right_splitter);
      //SetMinimumPaneSize(MIN_PANE_SIZE);
      wxPersistentRegisterAndRestore(this, "DatasetMultiView");

      // nested splitter contains grid and details
      m_listView = DatasetListView::create(m_right_splitter, source);
      m_details_panel = DetailsPanel::create(m_right_splitter, source, cache);
      m_right_splitter->SplitVertically(m_listView, m_details_panel);
      m_right_splitter->SetMinimumPaneSize(MIN_PANE_SIZE);
      wxPersistentRegisterAndRestore(m_right_splitter, "DatasetMultiViewNested");
      
      // For some reason the CalLAfter() is required, otherwise this call messes up the
      // next splitter layout. No idea why but this works.
      CallAfter([this] { m_right_splitter->SetSashGravity(RIGHT_SPLITTER_GRAVITY); });
   }


   auto DatasetMultiView::wineDetailsActive() const -> bool
   {
      return m_details_panel ? m_details_panel->wineDetailsActive() : false;
   }


} // namespace ctb::app
