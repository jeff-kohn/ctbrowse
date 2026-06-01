#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"

#include <wx/panel.h>
#include <deque>


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
      wxString            m_title{ constants::LBL_TASTING_NOTE };
      wxString            m_feedback_summary{};

      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailTastingPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& event_source) override // we don't add any detail rows in this panel.
      {}

      void onDatasetEvent(const DatasetEvent& event);
      void postWindowCreate() override;
   };


}
