#pragma once

#include "App.h"
#include "controls/WineDetailBasePanel.h"


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
      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args)->WndT*;

      WineDetailScorePanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source, constants::LBL_SCORES }
      {}

      // base class overrides
      void getDetailFields(DetailFields& fields) override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);

         fields.push_back(SinglePropDetailField{ top_sizer, CtProp::MyScore, constants::LBL_MY_SCORE }.setFormat(constants::FMT_NUMBER_DECIMAL).setNullDisplayValue(constants::NO_SCORE));
         fields.push_back(SinglePropDetailField{ top_sizer, CtProp::CtScore, constants::LBL_CT_SCORE }.setFormat(constants::FMT_NUMBER_DECIMAL).setNullDisplayValue(constants::NO_SCORE));
         fields.push_back(ProScoreSummaryField { top_sizer, constants::LBL_PRO_SCORES                });
      }
   };

}