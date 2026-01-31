#pragma once

#include "App.h"
#include "controls/WineDetailFields.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/panel.h>
#include <deque>


class wxStaticText;

namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailTastingPanel final : public wxPanel
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailTastingPanel*;

   private:
      DatasetEventHandler m_dataset_events;
      wxString            m_title{ constants::LBL_TASTING_NOTE };
      wxString            m_feedback_summary{};
      wxString            m_tasting_notes{};
      wxStaticText*       m_tasting_notes_ctrl{};

      WineDetailTastingPanel(const DatasetEventSourcePtr& event_source) : m_dataset_events{ event_source }
      {}

      void createWindow(wxWindow* parent);

      void onDatasetEvent(const DatasetEvent& event);
      void onSize(wxSizeEvent& event);
      void calcNoteSize();
   };


}