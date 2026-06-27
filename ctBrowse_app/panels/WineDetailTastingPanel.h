#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"

#include <deque>
#include <wx/panel.h>


class wxTextCtrl;

namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailTastingPanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailTastingPanel*
      {
         return detail::createDatasetWindow<WineDetailTastingPanel>(parent, source);
      }

   private:
      wxString m_title{ constants::LBL_TASTING_NOTE };
      wxString m_feedback_summary{};

      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailTastingPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void addDetails(DetailRows& rows, const DatasetEventSourcePtr& source) override;
      void onDatasetEvent(const DatasetEvent& event) override;
   };


}   // namespace ctb::app
