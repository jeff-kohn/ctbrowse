#include "WineDetailMainPanel.h"

#include <wx/stattext.h>
#include <wx/valgen.h>


namespace ctb::app
{
   auto WineDetailMainPanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*
   {
      return detail::createDatasetWindow<WineDetailMainPanel>(parent, source);
   }


   void WineDetailMainPanel::onDatasetEvent(const DatasetEvent& event)
   {
      if (event.affected_row.has_value())
      {
         m_wine_title = event.dataset->getProperty(event.affected_row.value(), CtProp::WineName).asString();
      }
      else
      {
         m_wine_title.clear();
      }
   }


   void WineDetailMainPanel::onSize(wxSizeEvent& event)
   {
      // Need to figure out how many lines to make the wine title so we can wrap it and show the full name
      constexpr auto margin = 5;
      if (event.m_size.x > 0)
      {
         m_wine_ctrl->SetLabelText(m_wine_title); // to remove any line feeds from previously-wrapped label.
         m_wine_ctrl->Wrap(event.m_size.GetWidth() - margin);
         wxSize best_size = m_wine_ctrl->GetBestSize();
         m_wine_ctrl->SetClientSize(best_size);
      }

      // Preserve default processing (important for proper propagation to parent/layout).
      event.Skip();
   }


   void WineDetailMainPanel::getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& source)
   {
      auto* top_sizer = GetSizer();
      assert(top_sizer);
      auto dataset = source->getDataset();
      assert(dataset);

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

   void WineDetailMainPanel::postWindowCreate()
   {
      m_wine_ctrl = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      m_wine_ctrl->SetValidator(wxGenericValidator(&m_wine_title));
      m_wine_ctrl->SetFont(GetFont().MakeLarger().MakeBold());
      m_wine_ctrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));

      auto* top_sizer = GetSizer();
      assert(top_sizer);
      top_sizer->Insert(0, m_wine_ctrl, wxSizerFlags{}.Center().Border());

      Fit();
      Layout();

      // handle resize so children are laid out correctly when this panel is resized
      Bind(wxEVT_SIZE, &WineDetailMainPanel::onSize, this);
   }

}   // namespace ctb::app
