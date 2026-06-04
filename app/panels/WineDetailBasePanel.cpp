#include "WineDetailBasePanel.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   void WineDetailBasePanel::onDatasetEventPrivate(const DatasetEvent& event)
   {
      // give derived classes a chance to do their thing.
      onDatasetEvent(event);

      // we want to update the detail rows' visibility AFTER all of the child windows have had a
      // chance to update, so we'll make a deferred call to do it.
      switch (event.event_id)
      {
         case DatasetEvent::Id::RowSelected:             [[fallthrough]];
         case DatasetEvent::Id::DatasetFiltered:         [[fallthrough]];
         case DatasetEvent::Id::DatasetSorted:           [[fallthrough]];
         case DatasetEvent::Id::DatasetSubStringFilter:
            CallAfter(&WineDetailBasePanel::updateVisibility);
            break;

         default: break;
      };
   }

   void WineDetailBasePanel::updateVisibility()
   {
      rng::for_each(m_rows, &WineDetailPanelRow::updateVisibility);

      InvalidateBestSize();     // force recalc since rows may have been shown/hidden
      PostSizeEventToParent();  // So parent can re-layout all the panels.
   }


   void WineDetailBasePanel::createWindow(wxWindow* parent)
   {
      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);

      // top level sizer contains the heading and the property grid of detail fields.
      auto* top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);

      // show a title (optional)
      if (!m_title.empty())
      {
         auto* heading_lbl = new wxStaticText(this, wxID_ANY, m_title, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
         heading_lbl->SetFont(GetFont().MakeBold());
         heading_lbl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
         top_sizer->Add(heading_lbl, wxSizerFlags{ 1 }.Expand().Border(wxBOTTOM | wxTOP));
      }

      addDetails(m_rows, getEventHandler().getSource());
      Layout();
      Fit();

      getEventHandler().setDefaultHandler([this](const DatasetEvent& event) { onDatasetEventPrivate(event); });
   }

} // namespace ctb::app
