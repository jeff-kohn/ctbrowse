#include "panels/WineDetailsPanel.h"

#include <cpr/cpr.h>
#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>

#include <wx/commandlinkbutton.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/wupdlock.h>

#include <format>
#include <chrono>
#include <thread>

namespace ctb::app
{

   namespace detail
   {
      wxString getDrinkWindow(const CtProperty& drink_start, const CtProperty& drink_end)
      {

         if (drink_start.isNull() && drink_end.isNull() )
            return wxEmptyString;

         if (drink_start.isNull() && !drink_end.isNull())
            return drink_end.asString("By {}").c_str();

         if (drink_end.isNull() )
            return drink_start.asString("{} +").c_str();

         return ctb::format("{} - {}", drink_start.asString(), drink_end.asString()).c_str(); 
      }


      void getLabelImage(uint64_t wine_id)
      {
         try 
         {
            // define the user agent for the GET request
            cpr::Header headers = { {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36"} };

            // make an HTTP GET request to retrieve the target page
            cpr::Response response = cpr::Get(cpr::Url{ ctb::format(constants::FMT_URL_WINE_DETAILS, wine_id) }, headers);

            HtmlParser::Parser parser;
            HtmlParser::DOM dom = parser.Parse(response.text);
            HtmlParser::Query query(dom.Root());
            auto images = dom.GetElementById("label_photo");
            if (images and !images->Children.empty())
            {
               auto image_url = images->Children[0]->GetAttribute("src");
               response = cpr::Get(cpr::Url{ image_url }, headers);
            }

         }
         catch (Error& err)
         {
            wxGetApp().displayErrorMessage(err);
         }
         catch (std::exception& e)
         {
            wxGetApp().displayErrorMessage(e.what());
         }
      }
   }


   WineDetailsPanel::WineDetailsPanel(GridTableEventSourcePtr source) : m_event_sink{ this, source }
   {}


   WineDetailsPanel* WineDetailsPanel::create(wxWindow* parent, GridTableEventSourcePtr source)
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

      std::unique_ptr<WineDetailsPanel> wnd{ new WineDetailsPanel{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release();
   }


   void WineDetailsPanel::initControls()
   {
      wxWindowUpdateLocker freeze_win(this);

      SetMaxSize(ConvertDialogToPixels( wxSize{220, -1} ));
      SetMinSize(ConvertDialogToPixels( wxSize{100, -1} ));

      const auto border_size = wxSizerFlags::GetDefaultBorder();

      auto default_font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
      default_font.SetPointSize(default_font.GetPointSize() + 1);
      auto title_font{ default_font.Bold() };
      auto wine_font{ default_font.Bold() };
      wine_font.SetPointSize(default_font.GetPointSize() + 1);

      const auto currency_min_size = ConvertDialogToPixels( wxSize{25, -1} );
      const auto currency_max_size = ConvertDialogToPixels( wxSize{30, -1} );

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
      auto* view_online_btn = new wxCommandLinkButton(this, wxID_ANY, "View Online at CellarTracker.com");
      top_sizer->Add(view_online_btn, wxSizerFlags().Border(wxALL).Expand());

      top_sizer->ShowItems(false);
      SetSizerAndFit(top_sizer);

      // hook up event handlers
      view_online_btn->Bind(wxEVT_BUTTON, &WineDetailsPanel::onViewWebPage, this);

   }


   void WineDetailsPanel::notify(GridTableEvent event)
   {
      try
      {
         switch (event.m_event_id)
         {
         case GridTableEvent::Id::RowSelected:
            UpdateDetails(event);
            break;

         case GridTableEvent::Id::TableInitialize:
            break;

         default:
            event.m_affected_row = std::nullopt;
            UpdateDetails(event);
            break;
         }
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }  
   }


   void WineDetailsPanel::UpdateDetails(GridTableEvent event)
   {
      using namespace magic_enum;
      
      wxWindowUpdateLocker freeze_win(this);

      if (event.m_affected_row and event.m_affected_row.value() >= 0)
      {
         auto* tbl = event.m_grid_table;
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

         // detail::getLabelImage(m_details.wine_id);
      }
      else{
         GetSizer()->ShowItems(false);
         m_details = WineDetails{};
      }

      TransferDataToWindow();
      Layout();
      // TODO hide/unhide fields as necessary.
   }

   void WineDetailsPanel::onViewWebPage([[maybe_unused]] wxCommandEvent& event)
   {
      if (!m_details.wine_id)
      {
         wxGetApp().displayInfoMessage("no wine id available.");
      }
      else{
         wxLaunchDefaultBrowser(ctb::format(constants::FMT_URL_WINE_DETAILS, m_details.wine_id).c_str());
      }
   }


} // namesapce ctb::app