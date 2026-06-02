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

      void addDetails(DetailRows& rows, const DatasetEventSourcePtr& source) override
      {
         // clang-format off
         auto* top_sizer = GetSizer();         assert(top_sizer);
         auto dataset = source->getDataset();  assert(dataset);

         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingStoreName ), constants::LBL_STORE_NAME);
         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingOrderQty  ), constants::LBL_QTY_ORDERED);

         rows.emplace_back(
            top_sizer,
            PropertyValueCtrl::create(this, source, CtProp::MyPrice, constants::FMT_NUMBER_CURRENCY, {}),
            constants::LBL_MY_PRICE
         );
         rows.emplace_back(
            top_sizer,
            PropertyValueCtrl::create(this, source, CtProp::PendingOrderDate, constants::FMT_NUMBER_CURRENCY, {}),
            constants::LBL_ORDER_DATE
         );
         rows.emplace_back(
            top_sizer,
            PropertyValueCtrl::create(this, source, CtProp::PendingDeliveryDate, constants::FMT_NUMBER_CURRENCY, {}),
            constants::LBL_DELIVERY_DATE
         );

         rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::PendingOrderNumber), constants::LBL_ORDER_NUMBER);
      }
   };


}   // namespace ctb::app
