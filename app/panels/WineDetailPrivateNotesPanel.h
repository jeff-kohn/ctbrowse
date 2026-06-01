#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"


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
      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailPrivateNotesPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& event_source) override  // we don't add any detail rows in this panel.
      {} 
      void postWindowCreate() override;
   };

}
