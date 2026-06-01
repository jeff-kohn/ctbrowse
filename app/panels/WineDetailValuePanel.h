#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailValuePanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailValuePanel*
      {
         return detail::createDatasetWindow<WineDetailValuePanel>(parent, source);
      }

   private:
      WineDetailValuePanel(const DatasetEventSourcePtr& source) : WineDetailBasePanel{ source, constants::LBL_VALUATION }
      {}

      DECLARE_DATASET_WINDOW_FACTORY;

      // base class overrides
      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source) override
      {
         auto* top_sizer = GetSizer();          assert(top_sizer);
         auto dataset = source->getDataset();   assert(dataset);

         // ordering matters here because it's the same as they'll be displayed
         auto* ctrl = PropertyValueCtrl::create(this, source, CtProp::MyPrice);
         ctrl->setFormat(constants::FMT_NUMBER_CURRENCY);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_STORE_NAME);

         ctrl = PropertyValueCtrl::create(this, source, CtProp::CtPrice);
         ctrl->setFormat(constants::FMT_NUMBER_CURRENCY);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_CT_PRICE);

         if (dataset->hasProperty(CtProp::AuctionPrice))
         {
            ctrl = PropertyValueCtrl::create(this, source, CtProp::AuctionPrice);
            ctrl->setFormat(constants::FMT_NUMBER_CURRENCY);
            rows.emplace_back(top_sizer, ctrl, constants::LBL_AUCTION_PRICE);
         }
      }
   };
}
