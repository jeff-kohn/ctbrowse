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
      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailPendingPanel(const DatasetEventSourcePtr& source) : WineDetailBasePanel{ source, constants::LBL_ORDER_DETAILS }
      {}

      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source) override
      {
         auto* top_sizer = GetSizer();         assert(top_sizer);
         auto dataset = source->getDataset();  assert(dataset);

         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingStoreName), constants::LBL_STORE_NAME);
         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingOrderQty), constants::LBL_QTY_ORDERED);

         auto* ctrl = PropertyValueCtrl::create(this, source, CtProp::MyPrice);
         ctrl->setFormat(constants::FMT_NUMBER_CURRENCY);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_MY_PRICE);

         ctrl = PropertyValueCtrl::create(this, source, CtProp::PendingOrderDate);
         ctrl->setFormat(constants::FMT_DATE_SHORT);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_ORDER_DATE);

         ctrl = PropertyValueCtrl::create(this, source, CtProp::PendingDeliveryDate);
         ctrl->setFormat(constants::FMT_DATE_SHORT);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_DELIVERY_DATE);

         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingOrderNumber), constants::LBL_ORDER_NUMBER);
      }
   };


}   // namespace ctb::app
