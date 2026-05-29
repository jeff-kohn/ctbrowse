#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"

class wxStaticText;


namespace ctb::app
{
   /// @brief A panel class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailMainPanel final : public WineDetailBasePanel
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*;

   private:
      wxString      m_wine_title{};
      wxStaticText* m_wine_ctrl{};

      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailMainPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      // base class overrides
      void getDetailFields(DetailFields& fields) override;
      void postWindowCreate() override;
      void onDatasetEvent(const DatasetEvent& event) override;

      // size event handler for wrapping wine title.
      void onSize(wxSizeEvent& event);
   };



}
