#pragma once

#include "App.h"
#include "controls/WineDetailBasePanel.h"

#include <wx/panel.h>
#include <deque>


class wxStaticText;

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
      wxString            m_tasting_notes{};
      wxStaticText*       m_tasting_notes_ctrl{};

      // this class can only be constructructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args... args)->WndT*;

      WineDetailTastingPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void getDetailFields(DetailFields& fields) override {} // we don't use DataFields in this panel.
      void onDatasetEvent(const DatasetEvent& event) override;
      void postWindowCreate() override;

      void onSize(wxSizeEvent& event);
      void calcNoteSize();
   };


}