#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"


namespace ctb::app
{

   class ElasticMultiLineTextCtrl;


   /// @brief A panel class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailMainPanel final : public WineDetailBasePanel
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*;

   private:
      wxString                  m_wine_title{};
      ElasticMultiLineTextCtrl* m_wine_ctrl{};

      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailMainPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      // base class overrides
      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source) override;
   };


}   // namespace ctb::app
