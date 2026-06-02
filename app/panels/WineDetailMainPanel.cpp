#include "WineDetailMainPanel.h"

#include "controls/ElasticMultiLineTextCtrl.h"

#include <wx/stattext.h>


namespace ctb::app
{
   auto WineDetailMainPanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*
   {
      return detail::createDatasetWindow<WineDetailMainPanel>(parent, source);
   }


   void WineDetailMainPanel::getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source)
   {
      auto* top_sizer = GetSizer();
      assert(top_sizer);
      auto dataset = source->getDataset();
      assert(dataset);

      m_wine_ctrl = ElasticMultiLineTextCtrl::create(this, source, CtProp::WineName, wxAlignment::wxALIGN_CENTER);
      m_wine_ctrl->SetFont(GetFont().MakeLarger().MakeBold());
      m_wine_ctrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));

      top_sizer->Insert(0, m_wine_ctrl, wxSizerFlags{}.Border().Expand());

      // clang-format off
      // ordering matters here because it's the same as they'll be displayed
      rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Vintage    ), constants::LBL_VINTAGE     );
      rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Varietal   ), constants::LBL_VARIETAL    );
      rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Country    ), constants::LBL_COUNTRY     );
      rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Region     ), constants::LBL_REGION      );
      rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::SubRegion  ), constants::LBL_SUB_REGION  );
      rows.emplace_back(top_sizer, PropertyValueCtrl::create(this, source, CtProp::Appellation), constants::LBL_APPELLATION );

      if (dataset->hasProperty(CtProp::CtBeginConsume))
      {
         rows.emplace_back(top_sizer, DrinkWindowCtrl::create(this, source, CtProp::BeginConsume,   CtProp::EndConsume   ), constants::LBL_DRINK_WINDOW_MY);
         rows.emplace_back(top_sizer, DrinkWindowCtrl::create(this, source, CtProp::CtBeginConsume, CtProp::CtEndConsume ), constants::LBL_DRINK_WINDOW_CT);
      }
      else
      {
         rows.emplace_back(top_sizer, DrinkWindowCtrl::create(this, source, CtProp::BeginConsume, CtProp::EndConsume), constants::LBL_DRINK_WINDOW);
      }
      // clang-format on

      rows.emplace_back(top_sizer, ProReviewsCacheCtrl::create(this, source, &ProReviewsCache::getDrinkWindowSummary), constants::LBL_DRINK_WINDOW_PRO);
   }


}   // namespace ctb::app
