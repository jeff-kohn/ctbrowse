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

      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args)->WndT*;

      WineDetailTastingPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void getDetailFields(DetailFields&) override {} // we don't use DataFields in this panel.
      void onDatasetEvent(const DatasetEvent& event) override;
      void postWindowCreate() override;
   };


}
