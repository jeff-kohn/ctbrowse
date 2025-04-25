/*********************************************************************
 * @file       DetailsPanel.cpp
 *
 * @brief      implementation for the DetailsPanel class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "views/DetailsPanel.h"

#include <ctb/utility_http.h>
#include <cpr/cpr.h>

#include <wx/commandlinkbutton.h>
#include <wx/collpane.h>
#include <wx/gdicmn.h>
#include <wx/hyperlink.h>
#include <wx/generic/hyperlink.h>
#include <wx/generic/statbmpg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/wupdlock.h>

#include <chrono>
#include <thread>

namespace ctb::constants
{
   constexpr auto LABEL_TIMER_RETRY_INTERVAL = 100; 

} // namespace constants

namespace ctb::app
{
   namespace detail
   {
      auto getDrinkWindow(const CtProperty& drink_start, const CtProperty& drink_end) -> wxString
      {
         if (drink_start.isNull() && drink_end.isNull() )
            return wxEmptyString;

         if (drink_start.isNull() && !drink_end.isNull())
            return drink_end.asString("By {}").c_str();

         if (drink_end.isNull() )
            return drink_start.asString("{} +").c_str();

         return ctb::format("{} - {}", drink_start.asString(), drink_end.asString()).c_str(); 
      }

   } // namespace detail


   DetailsPanel::DetailsPanel(DatasetEventSourcePtr source, LabelCachePtr cache) : 
      m_event_sink{ this, source },
      m_label_cache{ cache }
   {}


   DetailsPanel* DetailsPanel::create(wxWindow* parent, DatasetEventSourcePtr source, LabelCachePtr cache)
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!parent)
      {
         assert("parent parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<DetailsPanel> wnd{ new DetailsPanel{ source, cache } };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release();
   }


   void DetailsPanel::initControls()
   {
      wxWindowUpdateLocker freeze_win(this);

      SetMaxSize(ConvertDialogToPixels( wxSize{220, -1} ));
      SetMinSize(ConvertDialogToPixels( wxSize{100, -1} ));

      const auto border_size = wxSizerFlags::GetDefaultBorder();

      // configure font sizes/weights for property display
      auto default_font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
      default_font.SetPointSize(default_font.GetPointSize() + 1);
      auto title_font{ default_font.Bold() };
      auto wine_font{ default_font.Bold() };
      wine_font.SetPointSize(default_font.GetPointSize() + 1);

      const auto currency_min_size = ConvertDialogToPixels( wxSize{28, -1} );
      const auto currency_max_size = ConvertDialogToPixels( wxSize{36, -1} );

      auto* top_sizer = new wxBoxSizer(wxVERTICAL);

      // wine name is above the grid sizer so it can span both columns
      auto* wine_name_val = new wxStaticText(this, wxID_ANY, "");
      wine_name_val->SetValidator(wxGenericValidator(&m_details.wine_name));
      wine_name_val->SetMaxSize(ConvertDialogToPixels( wxSize{-1, 22} ));
      wine_name_val->SetFont(wine_font);
      top_sizer->Add(wine_name_val, wxSizerFlags{1}.Expand().Border(wxLEFT|wxRIGHT|wxTOP));

      // grid sizer gives us a property grid (eg a column of labels and values)
      auto* details_sizer = new wxFlexGridSizer(2, 0, 0);

      // vintage
      auto* lbl_vintage = new wxStaticText(this, wxID_ANY, constants::LBL_VINTAGE);
      lbl_vintage->SetFont(default_font);
      details_sizer->Add(lbl_vintage, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* vintage_val = new wxStaticText(this, wxID_ANY, "");
      vintage_val->SetValidator(wxGenericValidator{ &m_details.vintage });
      vintage_val->SetFont(default_font);
      details_sizer->Add(vintage_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // varietal
      auto* lbl_varietal = new wxStaticText(this, wxID_ANY, constants::LBL_VARIETAL);
      lbl_varietal->SetFont(default_font);
      details_sizer->Add(lbl_varietal, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* varietal_val = new wxStaticText(this, wxID_ANY, "");
      varietal_val->SetFont(default_font);
      varietal_val->SetValidator(wxGenericValidator{ &m_details.varietal });
      details_sizer->Add(varietal_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));


      // country
      auto* lbl_country = new wxStaticText(this, wxID_ANY, constants::LBL_COUNTRY);
      lbl_country->SetFont(default_font);
      details_sizer->Add(lbl_country, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* country_region_val = new wxStaticText(this, wxID_ANY, "");
      country_region_val->SetValidator(wxGenericValidator{ &m_details.country });
      country_region_val->SetFont(default_font);
      details_sizer->Add(country_region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // region
      auto* lbl_region = new wxStaticText(this, wxID_ANY, constants::LBL_REGION);
      lbl_region->SetFont(default_font);
      details_sizer->Add(lbl_region, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* region_val = new wxStaticText(this, wxID_ANY, "");
      region_val->SetValidator(wxGenericValidator{ &m_details.region });
      region_val->SetFont(default_font);
      details_sizer->Add(region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // subregion
      auto* lbl_sub_region = new wxStaticText(this, wxID_ANY, constants::LBL_SUB_REGION);
      lbl_sub_region->SetFont(default_font);
      details_sizer->Add(lbl_sub_region, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* sub_region_val = new wxStaticText(this, wxID_ANY, "");
      sub_region_val->SetValidator(wxGenericValidator{ &m_details.sub_region });
      sub_region_val->SetFont(default_font);
      details_sizer->Add(sub_region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // appellation
      auto* lbl_appellation = new wxStaticText(this, wxID_ANY, constants::LBL_APPELLATION);
      lbl_appellation->SetFont(default_font);
      details_sizer->Add(lbl_appellation, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* appellation_val = new wxStaticText(this, wxID_ANY, "");
      appellation_val->SetValidator(wxGenericValidator{ &m_details.appellation });
      appellation_val->SetFont(default_font);
      details_sizer->Add(appellation_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // drink window
      auto* lbl_drink_window = new wxStaticText(this, wxID_ANY, constants::LBL_DRINK_WINDOW);
      lbl_drink_window->SetFont(default_font);
      details_sizer->Add(lbl_drink_window, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* drink_window_val = new wxStaticText(this, wxID_ANY, "");
      drink_window_val->SetFont(default_font);
      drink_window_val->SetValidator(wxGenericValidator{ &m_details.drink_window });
      details_sizer->Add(drink_window_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // Scores heading
      auto* lbl_scores_header = new wxStaticText(this, wxID_ANY, constants::LBL_SCORES);
      lbl_scores_header->SetFont(title_font);
      details_sizer->Add(lbl_scores_header, wxSizerFlags{}.Border(wxALL));
      details_sizer->AddSpacer(0);

      // My Score
      auto* lbl_my_score = new wxStaticText(this, wxID_ANY, constants::LBL_MY_SCORE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_my_score->SetFont(default_font);
      details_sizer->Add(lbl_my_score, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* my_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_score_val->SetFont(default_font);
      my_score_val->SetValidator(wxGenericValidator(&m_details.my_score));
      details_sizer->Add(my_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // CT Score
      auto* lbl_ct_score = new wxStaticText(this, wxID_ANY, constants::LBL_CT_SCORE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_ct_score->SetFont(default_font);
      details_sizer->Add(lbl_ct_score, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* ct_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_score_val->SetFont(default_font);
      ct_score_val->SetValidator(wxGenericValidator{ &m_details.ct_score });
      details_sizer->Add(ct_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // Valuation heading
      auto* lbl_value_header = new wxStaticText(this, wxID_ANY, constants::LBL_VALUATION);
      lbl_value_header->SetFont(title_font);
      details_sizer->Add(lbl_value_header, wxSizerFlags{}.Border(wxALL));
      details_sizer->AddSpacer(0);

      // My Price
      auto* lbl_my_price = new wxStaticText(this, wxID_ANY, constants::LBL_MY_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_my_price->SetFont(default_font);
      details_sizer->Add(lbl_my_price, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* my_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_price_val->SetFont(default_font);
      my_price_val->SetMinSize(currency_min_size);
      my_price_val->SetMaxSize(currency_max_size);
      my_price_val->SetValidator(wxGenericValidator{ &m_details.my_price });
      details_sizer->Add(my_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // Community Avg
      auto* lbl_ct_price = new wxStaticText(this, wxID_ANY, constants::LBL_CT_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_ct_price->SetFont(default_font);
      details_sizer->Add(lbl_ct_price, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* ct_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_price_val->SetFont(default_font);
      ct_price_val->SetMinSize(currency_min_size);
      ct_price_val->SetMaxSize(currency_max_size);
      ct_price_val->SetValidator(wxGenericValidator{ &m_details.community_price });
      details_sizer->Add(ct_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // Auction value
      auto* lbl_auction_value = new wxStaticText(this, wxID_ANY, constants::LBL_AUCTION_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_auction_value->SetFont(default_font);
      details_sizer->Add(lbl_auction_value, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* auction_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      auction_price_val->SetFont(default_font);
      auction_price_val->SetMinSize(currency_min_size);
      auction_price_val->SetMaxSize(currency_max_size);
      auction_price_val->SetValidator(wxGenericValidator{ &m_details.auction_value });
      details_sizer->Add(auction_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      top_sizer->Add(details_sizer, wxSizerFlags{}.Expand().FixedMinSize().Border(wxALL));

      // View Online button (also outside grid sizer, same as wine name)
      auto* view_online_btn = new wxCommandLinkButton(this, wxID_ANY, constants::DETAIL_VIEW_ONLINE_TITLE, constants::DETAIL_VIEW_ONLINE_NOTE);
      top_sizer->Add(view_online_btn, wxSizerFlags().CenterHorizontal().Border(wxALL));

      // image won't correctly scale/redraw unless we use wxFULL_REPAINT_ON_RESIZE
      m_label_image = new wxGenericStaticBitmap(this, wxID_ANY, wxNullBitmap , wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
      m_label_image->SetScaleMode(wxStaticBitmap::Scale_AspectFit);
      top_sizer->Add(m_label_image, wxSizerFlags().CenterHorizontal().Expand().Shaped().Border(wxALL));

      SetSizerAndFit(top_sizer);
      top_sizer->ShowItems(false);

      // hook up event handlers
      m_label_timer.Bind(wxEVT_TIMER, &DetailsPanel::onLabelTimer, this);
      view_online_btn->Bind(wxEVT_BUTTON, &DetailsPanel::onViewWebPage, this);
   }


   void app::DetailsPanel::checkLabelResult()
   {
      using namespace tasks;

      if (auto& result = m_details.image_result)
      {
         switch (result->poll(0ms))
         {
            case wxImageTask::Status::Deferred: [[fallthrough]];
            case wxImageTask::Status::Finished:
               displayLabel();
               [[fallthrough]];

            case wxImageTask::Status::Invalid:
               m_details.image_result = {};
               break;

            case wxImageTask::Status::Running:
               m_label_timer.StartOnce(constants::LABEL_TIMER_RETRY_INTERVAL);
               break;

            default:
               assert("Bug, new enum value wasn't accounted for" and false);
               break;
         }
      }
   }


   void app::DetailsPanel::displayLabel()
   {
      try
      {
         if (m_details.image_result)
         {
            auto result = m_details.image_result->getImage();
            if (!result)
               throw result.error();

            wxBitmap bmp{ *result };
            m_label_image->SetBitmap(bmp);    
            m_label_image->Show();
            Layout(); // required since the images vary in size
            m_label_image->Refresh();
            m_label_image->Update();
         }
      }
      catch (...)
      {
         m_label_image->SetBitmap(wxBitmap{});
         m_label_image->Hide();
         Refresh();
         Update();
         log::exception(packageError());
      }
   }


   void DetailsPanel::updateDetails(DatasetEvent event)
   {
      using namespace magic_enum;
      
      wxWindowUpdateLocker freeze_win(this);

      if (event.m_affected_row and event.m_affected_row.value() >= 0)
      {
         auto* tbl = event.m_data.get();
         assert(tbl != nullptr);
         auto row_idx = event.m_affected_row.value();

         m_details.wine_id     = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_WINE_ID    ).asUInt64().value_or(0);
         m_details.wine_name   = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_WINE_NAME  ).asString();
         m_details.vintage     = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_VINTAGE    ).asString();
         m_details.varietal    = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_VARIETAL   ).asString();
         m_details.country     = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_COUNTRY    ).asString();
         m_details.region      = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_REGION     ).asString();
         m_details.sub_region  = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_SUB_REGION ).asString();
         m_details.appellation = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_APPELLATION).asString();

         m_details.drink_window     = detail::getDrinkWindow(tbl->getDetailProp(row_idx, constants::DETAIL_PROP_DRINK_START),
                                                             tbl->getDetailProp(row_idx, constants::DETAIL_PROP_DRINK_END));

         m_details.auction_value    = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_AUCTION_VALUE  ).asString(constants::FMT_NUMBER_CURRENCY).c_str();
         m_details.community_price  = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_COMMUNITY_PRICE).asString(constants::FMT_NUMBER_CURRENCY).c_str();
         m_details.my_price         = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_MY_PRICE       ).asString(constants::FMT_NUMBER_CURRENCY).c_str();

         auto prop_val = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_CT_SCORE);
         m_details.ct_score = prop_val ? prop_val.asString(constants::FMT_NUMBER_DECIMAL).c_str() : constants::NO_SCORE;

         prop_val = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_MY_SCORE);
         m_details.my_score = prop_val ? prop_val.asString(constants::FMT_NUMBER_DECIMAL).c_str() : constants::NO_SCORE;
         GetSizer()->ShowItems(true);

         m_label_image->Hide();
         m_details.image_result = m_label_cache->fetchLabelImage(m_details.wine_id);
         checkLabelResult();
      }
      else{
         GetSizer()->ShowItems(false);
         m_details = WineDetails{};
      }

      TransferDataToWindow();
      Layout();
      // TODO hide/unhide fields as necessary.
   }


   void DetailsPanel::notify(DatasetEvent event)
   {
      try
      {
         switch (event.m_event_id)
         {
         case DatasetEvent::Id::RowSelected:
            updateDetails(event);
            break;

         case DatasetEvent::Id::ColLayoutRequested: [[fallthrough]];
         case DatasetEvent::Id::TableInitialize:
            break;

         default:
            event.m_affected_row = std::nullopt;
            updateDetails(event);
            break;
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DetailsPanel::onLabelTimer(wxTimerEvent&)
   {
      checkLabelResult();
   }


   void DetailsPanel::onViewWebPage([[maybe_unused]] wxCommandEvent& event)
   {
      if (!m_details.wine_id)
      {
         wxGetApp().displayInfoMessage("no wine id available.");
      }
      else{
         wxLaunchDefaultBrowser(getWineDetailsUrl(m_details.wine_id).c_str());
      }
   }


} // namesapce ctb::app