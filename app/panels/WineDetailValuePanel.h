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
      WineDetailValuePanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source, constants::LBL_VALUATION }
      {}

      DECLARE_DATASET_WINDOW_FACTORY;

      // base class overrides
      void getDetailFields(DetailFields& fields) override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);
         auto dataset = getDataset();

         // ordering matters here because it's the same as they'll be displayed
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::MyPrice,      constants::LBL_MY_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));
         fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::CtPrice,      constants::LBL_CT_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));

         if (dataset->hasProperty(CtProp::AuctionPrice))
         {
            fields.emplace_back(SinglePropertyDisplay{ top_sizer, CtProp::AuctionPrice, constants::LBL_AUCTION_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));
         }
      }
   };
}
