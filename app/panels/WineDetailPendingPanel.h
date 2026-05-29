#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailPendingPanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailPendingPanel*
      {
         return detail::createDatasetWindow<WineDetailPendingPanel>(parent, source);
      }
   private:

      WineDetailPendingPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source, constants::LBL_ORDER_DETAILS }
      {}

      DECLARE_DATASET_WINDOW_FACTORY;

      void getDetailFields(DetailFields& fields) override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);
         auto dataset = getDataset();

         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingStoreName,    constants::LBL_STORE_NAME });
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingOrderQty,     constants::LBL_QTY_ORDERED });
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::MyPrice,             constants::LBL_MY_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingOrderDate,    constants::LBL_ORDER_DATE }.setFormat(constants::FMT_DATE_SHORT));
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingDeliveryDate, constants::LBL_DELIVERY_DATE }.setFormat(constants::FMT_DATE_SHORT));
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::PendingOrderNumber,  constants::LBL_ORDER_NUMBER });
      }

   };


}
