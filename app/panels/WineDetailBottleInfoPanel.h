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
      void getDetailFields(DetailFields& fields) override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);
         auto dataset = getDataset();

         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::Size,              constants::LBL_SIZE });
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::Location,          constants::LBL_LOCATION });
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::Bin,               constants::LBL_BIN });

         if (dataset->hasProperty(CtProp::PendingOrderDate))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingOrderDate,  constants::LBL_PURCHASED }.setFormat(constants::FMT_DATE_SHORT));
         }
         if (dataset->hasProperty(CtProp::PendingStoreName))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingStoreName,  constants::LBL_FROM });
         }
         if (dataset->hasProperty(CtProp::ConsumeDate))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::ConsumeDate,    constants::LBL_CONSUME_DATE });
         }
         if (dataset->hasProperty(CtProp::ConsumeReason))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::ConsumeReason,  constants::LBL_CONSUME_REASON });
         }
         if (dataset->hasProperty(CtProp::BottleNote))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::BottleNote, constants::LBL_BOTTLE_NOTE });
         }
         if (dataset->hasProperty(CtProp::ConsumeNote))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::ConsumeNote, constants::LBL_CONSUME_NOTE });
         }
      }
   };
}
