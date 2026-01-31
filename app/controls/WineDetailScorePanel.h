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
   class WineDetailScorePanel final : public wxPanel
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailScorePanel*;

   private:
      using DetailFields = std::deque<SinglePropDetailField>;

      DatasetEventHandler m_event_handler;
      DetailFields        m_fields{};
      wxString            m_title{ constants::LBL_SCORES };

      WineDetailScorePanel(const DatasetEventSourcePtr& event_source) : m_event_handler{ event_source }
      {}

      void createWindow(wxWindow* parent);
      void onDatasetEvent(const DatasetEvent& event);
   };


}