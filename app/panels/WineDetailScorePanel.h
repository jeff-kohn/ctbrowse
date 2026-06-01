#pragma once

#include "App.h"
#include "panels/WineDetailBasePanel.h"


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailScorePanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailScorePanel*
      {
         return detail::createDatasetWindow<WineDetailScorePanel>(parent, source);
      }

   private:
      DECLARE_DATASET_WINDOW_FACTORY;

      WineDetailScorePanel(const DatasetEventSourcePtr& source) : WineDetailBasePanel{ source, constants::LBL_SCORES }
      {}

      // base class overrides
      void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source) override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);

         auto* ctrl = PropertyValueCtrl::create(this, source, CtProp::MyScore);
         ctrl->setFormat(constants::FMT_NUMBER_DECIMAL);
         ctrl->setNullDisplayValue(constants::NO_SCORE);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_MY_SCORE);

         ctrl = PropertyValueCtrl::create(this, source, CtProp::CtScore);
         ctrl->setFormat(constants::FMT_NUMBER_DECIMAL);
         ctrl->setNullDisplayValue(constants::NO_SCORE);
         rows.emplace_back(top_sizer, ctrl, constants::LBL_CT_SCORE);

         rows.emplace_back(top_sizer, ProReviewsCacheCtrl::create(this, source, &ProReviewsCache::getScoreSummary), constants::LBL_PRO_SCORES);
      }
   };

}
