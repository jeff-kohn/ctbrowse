#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a bottle
   ///
   class WineDetailBottleInfoPanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailBottleInfoPanel*
      {
         return detail::createDatasetWindow<WineDetailBottleInfoPanel>(parent, source);
      }

   private:
      WineDetailBottleInfoPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source, constants::LBL_BOTTLE_INFO }
      {}

      DECLARE_DATASET_WINDOW_FACTORY;

      // base class overrides
      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source) override
      {
         auto* top_sizer = GetSizer();  assert(top_sizer);
         auto  dataset   = source->getDataset();

         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Size    ), constants::LBL_SIZE);
         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Location), constants::LBL_LOCATION);
         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Bin), constants::LBL_BIN);

         if (dataset->hasProperty(CtProp::PendingOrderDate))
         {
            rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingOrderDate), constants::LBL_PURCHASED);
         }
         if (dataset->hasProperty(CtProp::PendingStoreName))
         {
            rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingStoreName), constants::LBL_FROM);
         }
         if (dataset->hasProperty(CtProp::ConsumeDate))
         {
            rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::ConsumeDate), constants::LBL_CONSUME_DATE);
         }
         if (dataset->hasProperty(CtProp::ConsumeReason))
         {
            rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::ConsumeReason), constants::LBL_CONSUME_REASON);
         }
         if (dataset->hasProperty(CtProp::BottleNote))
         {
            rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::BottleNote), constants::LBL_BOTTLE_NOTE);
         }
         if (dataset->hasProperty(CtProp::ConsumeNote))
         {
            rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::ConsumeNote), constants::LBL_CONSUME_NOTE);
         }
      }
   };
}   // namespace ctb::app
