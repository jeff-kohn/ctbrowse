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
      WineDetailTastingPanel(wxWindow* parent, const DatasetEventSourcePtr& event_source);

   private:
      DatasetEventHandler m_event_handler;
      wxString            m_title{ constants::LBL_TASTING_NOTE };
      wxString            m_feedback_summary{};
      wxString            m_tasting_notes{};
      wxStaticText*       m_tasting_notes_ctrl{};

      void init();
      void onDatasetEvent(const DatasetEvent& event);
   
      // size event handler
      void onSize(wxSizeEvent& event);
   };


}