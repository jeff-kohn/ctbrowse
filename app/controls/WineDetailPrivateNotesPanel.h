#pragma once

#include "App.h"
#include "controls/WineDetailBasePanel.h"


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailPrivateNotesPanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailPrivateNotesPanel*
      {
         return detail::createDatasetWindow<WineDetailPrivateNotesPanel>(parent, source);
      }

   private:
      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args)->WndT*;

      WineDetailPrivateNotesPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void getDetailFields(DetailFields&) override {} // we don't use DataFields in this panel.
      void postWindowCreate() override;
   };

}
