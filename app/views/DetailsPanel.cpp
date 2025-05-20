/*********************************************************************
 * @file       DetailsPanel.cpp
 *
 * @brief      implementation for the DetailsPanel class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "views/DetailsPanel.h"

#include <ctb/utility_http.h>

#include <wx/commandlinkbutton.h>
#include <wx/collpane.h>
#include <wx/gdicmn.h>
#include <wx/hyperlink.h>
#include <wx/generic/statbmpg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/wupdlock.h>


namespace ctb::constants
{
   constexpr auto LABEL_TIMER_RETRY_INTERVAL = 33; 

} // namespace constants


namespace ctb::app
{
   namespace detail
   {
      auto getDrinkWindow(const CtProperty& drink_start, const CtProperty& drink_end) -> wxString
      {
         if (drink_start.isNull() && drink_end.isNull())
            return wxEmptyString;

         if (drink_start.isNull() && !drink_end.isNull())
            return drink_end.asString("By {}").c_str();

         if (drink_end.isNull())
            return drink_start.asString("{} +").c_str();

         return wxString::FromUTF8(ctb::format("{} - {}", drink_start.asString(), drink_end.asString()).c_str());
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


   auto DetailsPanel::wineDetailsActive() const -> bool
   {
      return GetSizer()->AreAnyItemsShown();
   }


   void DetailsPanel::initControls()
   {
      using enum CategorizedControls::Category;

      wxWindowUpdateLocker freeze_win(this);

      SetMaxSize(ConvertDialogToPixels( wxSize{220, -1} ));
      SetMinSize(ConvertDialogToPixels( wxSize{100, -1} ));

      const auto border_size = wxSizerFlags::GetDefaultBorder();
      const auto heading_color = wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT);

      // configure font sizes/weights for property display
      auto title_font{ GetFont().MakeBold() };
      auto wine_font{ GetFont().MakeLarger().MakeBold()};

      auto* top_sizer = new wxBoxSizer(wxVERTICAL);

      // wine name is above the grid sizer so it can span both columns
      auto* wine_name_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      wine_name_val->SetValidator(wxGenericValidator(&m_details.wine_name));
      wine_name_val->SetMaxSize(ConvertDialogToPixels( wxSize{-1, 22} ));
      wine_name_val->SetFont(wine_font);
      wine_name_val->SetForegroundColour(heading_color);

      top_sizer->Add(wine_name_val, wxSizerFlags{2}.Expand().Border(wxRIGHT|wxTOP));

      // grid sizer gives us a property grid (eg a column of labels and values)
      auto* details_sizer = new wxFlexGridSizer(2, 0, 0);

      // vintage
      auto* vintage_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_VINTAGE);
      details_sizer->Add(vintage_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* vintage_val = new wxStaticText(this, wxID_ANY, "");
      vintage_val->SetValidator(wxGenericValidator{ &m_details.vintage });
      details_sizer->Add(vintage_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));

      // varietal
      auto* varietal_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_VARIETAL);
      details_sizer->Add(varietal_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* varietal_val = new wxStaticText(this, wxID_ANY, "");
      varietal_val->SetValidator(wxGenericValidator{ &m_details.varietal });
      details_sizer->Add(varietal_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));


      // country
      auto* country_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_COUNTRY);
      details_sizer->Add(country_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* country_region_val = new wxStaticText(this, wxID_ANY, "");
      country_region_val->SetValidator(wxGenericValidator{ &m_details.country });
      details_sizer->Add(country_region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));

      // region
      auto* region_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_REGION);
      details_sizer->Add(region_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* region_val = new wxStaticText(this, wxID_ANY, "");
      region_val->SetValidator(wxGenericValidator{ &m_details.region });
      details_sizer->Add(region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));

      // subregion
      auto* sub_region_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_SUB_REGION);
      details_sizer->Add(sub_region_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* sub_region_val = new wxStaticText(this, wxID_ANY, "");
      sub_region_val->SetValidator(wxGenericValidator{ &m_details.sub_region });
      details_sizer->Add(sub_region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));

      // appellation
      auto* appellation_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_APPELLATION);
      details_sizer->Add(appellation_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* appellation_val = new wxStaticText(this, wxID_ANY, "");
      appellation_val->SetValidator(wxGenericValidator{ &m_details.appellation });
      details_sizer->Add(appellation_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));

      // drink window
      auto* drink_window_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_DRINK_WINDOW);
      details_sizer->Add(drink_window_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* drink_window_val = new wxStaticText(this, wxID_ANY, "");
      drink_window_val->SetValidator(wxGenericValidator{ &m_details.drink_window });
      details_sizer->Add(drink_window_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));
      m_control_categories.addControlDependency(DrinkBy, drink_window_lbl);
      m_control_categories.addControlDependency(DrinkBy, drink_window_val);

      // Scores heading
      auto* scores_header_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_SCORES);
      scores_header_lbl->SetFont(title_font);
      scores_header_lbl->SetForegroundColour(heading_color);
      details_sizer->Add(scores_header_lbl, wxSizerFlags{}.Border(wxALL));
      details_sizer->AddSpacer(0);
      m_control_categories.addControlDependency(Score, scores_header_lbl);

      // My Score
      auto* my_score_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_MY_SCORE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(my_score_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* my_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_score_val->SetValidator(wxGenericValidator(&m_details.my_score));
      details_sizer->Add(my_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_control_categories.addControlDependency(Score, my_score_lbl);
      m_control_categories.addControlDependency(Score, my_score_val);

      // CT Score
      auto* ct_score_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_CT_SCORE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(ct_score_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* ct_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_score_val->SetValidator(wxGenericValidator{ &m_details.ct_score });
      details_sizer->Add(ct_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_control_categories.addControlDependency(Score, ct_score_lbl);
      m_control_categories.addControlDependency(Score, ct_score_val);

      // Valuation heading
      auto* value_header_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_VALUATION);
      value_header_lbl->SetFont(title_font);
      value_header_lbl->SetForegroundColour(heading_color);
      details_sizer->Add(value_header_lbl, wxSizerFlags{}.Border(wxALL));
      details_sizer->AddSpacer(0);
      m_control_categories.addControlDependency(Valuation, value_header_lbl);

      // My Price
      auto* my_price_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_MY_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(my_price_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* my_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_price_val->SetValidator(wxGenericValidator{ &m_details.my_price });
      details_sizer->Add(my_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_control_categories.addControlDependency(Valuation, my_price_lbl);
      m_control_categories.addControlDependency(Valuation, my_price_val);

      // Community Avg
      auto* ct_price_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_CT_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(ct_price_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* ct_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_price_val->SetValidator(wxGenericValidator{ &m_details.community_price });
      details_sizer->Add(ct_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_control_categories.addControlDependency(Valuation, ct_price_lbl);
      m_control_categories.addControlDependency(Valuation, ct_price_val);

      // Auction value
      auto* auction_value_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_AUCTION_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(auction_value_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* auction_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      auction_price_val->SetValidator(wxGenericValidator{ &m_details.auction_value });
      details_sizer->Add(auction_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_control_categories.addControlDependency(Valuation, auction_value_lbl);
      m_control_categories.addControlDependency(Valuation, auction_value_lbl);

      top_sizer->Add(details_sizer, wxSizerFlags{}.CenterHorizontal().FixedMinSize().Border(wxALL));

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
            SendSizeEvent();
            Update();
         }
      }
      catch (...)
      {
         log::exception(packageError());
         m_label_image->SetBitmap(wxBitmap{});
         m_label_image->Hide();
         Refresh();
         Update();
      }
   }


   void DetailsPanel::updateDetails(DatasetEvent event)
   {
      using namespace magic_enum;
      
      wxWindowUpdateLocker freeze_win(this);

      if (event.affected_row and event.affected_row.value() >= 0)
      {
         auto dataset = event.dataset;
         assert(dataset and "This pointer should never be null here!!!");

         auto rec_idx = event.affected_row.value();

         m_details.wine_id     = dataset->getProperty(rec_idx, CtProp::iWineId     ).asUInt64().value_or(0);
         m_details.wine_name   = dataset->getProperty(rec_idx, CtProp::WineName    ).asString();
         m_details.vintage     = dataset->getProperty(rec_idx, CtProp::Vintage     ).asString();
         m_details.varietal    = dataset->getProperty(rec_idx, CtProp::Varietal    ).asString();
         m_details.country     = dataset->getProperty(rec_idx, CtProp::Country     ).asString();
         m_details.region      = dataset->getProperty(rec_idx, CtProp::Region      ).asString();
         m_details.sub_region  = dataset->getProperty(rec_idx, CtProp::SubRegion   ).asString();
         m_details.appellation = dataset->getProperty(rec_idx, CtProp::Appellation ).asString();

         m_details.drink_window     = detail::getDrinkWindow(dataset->getProperty(rec_idx, CtProp::BeginConsume ),
                                                             dataset->getProperty(rec_idx, CtProp::EndConsume   ));

         m_details.auction_value    = dataset->getProperty(rec_idx, CtProp::AuctionPrice ).asString(constants::FMT_NUMBER_CURRENCY).c_str();
         m_details.community_price  = dataset->getProperty(rec_idx, CtProp::CtPrice      ).asString(constants::FMT_NUMBER_CURRENCY).c_str();
         m_details.my_price         = dataset->getProperty(rec_idx, CtProp::MyPrice      ).asString(constants::FMT_NUMBER_CURRENCY).c_str();

         auto prop_val = dataset->getProperty(rec_idx, CtProp::CtScore);
         m_details.ct_score = prop_val ? prop_val.asString(constants::FMT_NUMBER_DECIMAL).c_str() : constants::NO_SCORE;

         prop_val = dataset->getProperty(rec_idx, CtProp::MyScore);
         m_details.my_score = prop_val ? prop_val.asString(constants::FMT_NUMBER_DECIMAL).c_str() : constants::NO_SCORE;

         // show everything since detail panel may be blank if no record was selected previously
         GetSizer()->ShowItems(true); 
         // but show/hide control categories as appropriate.
         configureControlsForDataset(dataset);

         m_label_image->Hide(); // always starts hidden until we have an image
         m_details.image_result = m_label_cache->fetchLabelImage(m_details.wine_id);
         checkLabelResult();
      }
      else{
         GetSizer()->ShowItems(false);
         m_details = WineDetails{};
      }
      TransferDataToWindow();
      Layout();
      SendSizeEvent();
      Update();
   }


   void DetailsPanel::notify(DatasetEvent event)
   {
      try
      {
         switch (event.event_id)
         {
         case DatasetEvent::Id::RowSelected:
            updateDetails(event);
            break;

         case DatasetEvent::Id::ColLayoutRequested: [[fallthrough]];
         case DatasetEvent::Id::DatasetInitialize:
            break;

         default:
            event.affected_row = std::nullopt;
            updateDetails(event);
            break;
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }

   void DetailsPanel::configureControlsForDataset(DatasetPtr dataset)
   {
      using enum CategorizedControls::Category;
      m_control_categories.showCategory(Score,     dataset->hasProperty(CtProp::CtScore));
      m_control_categories.showCategory(DrinkBy,   dataset->hasProperty(CtProp::BeginConsume));
      m_control_categories.showCategory(Valuation, dataset->hasProperty(CtProp::MyPrice));
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