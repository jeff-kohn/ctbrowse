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
   class WineDetailValuePanel final : public wxPanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailValuePanel*;

   private:
      using DetailFields = std::deque<SinglePropDetailField>;

      DatasetEventHandler m_dataset_events;
      DetailFields        m_fields{};
      wxString            m_title{ constants::LBL_SCORES };

      WineDetailValuePanel(const DatasetEventSourcePtr& event_source) : m_dataset_events{ event_source }
      {}

      void createWindow(wxWindow* parent);
      void onDatasetEvent(const DatasetEvent& event);
   };
}