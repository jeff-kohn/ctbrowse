#pragma once

#include "App.h"
#include "controls/WineDetailFields.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/panel.h>
#include <deque>


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailPendingPanel final : public wxPanel
   {
   public:
      WineDetailPendingPanel(wxWindow* parent, DatasetEventSourcePtr event_source);

   private:
      using DetailFields = std::deque<SinglePropDetailField>;

      DatasetEventHandler m_event_handler;
      DetailFields        m_fields{};
      wxString            m_title{ constants::LBL_SCORES };

      void init();
      void onDatasetEvent(const DatasetEvent& event);
   };


}