#include "panels/WineDetailsPanel.h"

#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/wupdlock.h>

#include <format>


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

         return std::format("{} - {}", drink_start.asString(), drink_end.asString()).c_str();
      }
   }


   WineDetailsPanel::WineDetailsPanel(GridTableEventSourcePtr source) : m_event_sink{ this, source }
   {}


   WineDetailsPanel* ctb::app::WineDetailsPanel::create(wxWindow* parent, GridTableEventSourcePtr source)
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

      auto* details_sizer = new wxFlexGridSizer(2, 0, 0);

      // vintage
      auto* lbl_vintage = new wxStaticText(this, wxID_ANY, "Vintage:");
      lbl_vintage->SetFont(default_font);
      details_sizer->Add(lbl_vintage, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* vintage_val = new wxStaticText(this, wxID_ANY, "");
      vintage_val->SetValidator(wxGenericValidator{ &m_details.vintage });
      vintage_val->SetFont(default_font);
      details_sizer->Add(vintage_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // country
      auto* lbl_country_region = new wxStaticText(this, wxID_ANY, "Country:");
      lbl_country_region->SetFont(default_font);
      details_sizer->Add(lbl_country_region, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* country_region_val = new wxStaticText(this, wxID_ANY, "");
      country_region_val->SetValidator(wxGenericValidator{ &m_details.country });
      country_region_val->SetFont(default_font);
      details_sizer->Add(country_region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // region
      auto* lbl_region = new wxStaticText(this, wxID_ANY, "Region:");
      lbl_region->SetFont(default_font);
      details_sizer->Add(lbl_region, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* region_val = new wxStaticText(this, wxID_ANY, "");
      region_val->SetValidator(wxGenericValidator{ &m_details.region });
      region_val->SetFont(default_font);
      details_sizer->Add(region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // subregion
      auto* lbl_sub_region = new wxStaticText(this, wxID_ANY, "Subregion:");
      lbl_sub_region->SetFont(default_font);
      details_sizer->Add(lbl_sub_region, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* sub_region_val = new wxStaticText(this, wxID_ANY, "");
      sub_region_val->SetValidator(wxGenericValidator{ &m_details.sub_region });
      sub_region_val->SetFont(default_font);
      details_sizer->Add(sub_region_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // appellation
      auto* lbl_appellation = new wxStaticText(this, wxID_ANY, "Appellation:");
      lbl_appellation->SetFont(default_font);
      details_sizer->Add(lbl_appellation, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* appellation_val = new wxStaticText(this, wxID_ANY, "");
      appellation_val->SetFont(default_font);
      appellation_val->SetValidator(wxGenericValidator{ &m_details.appellation });
      details_sizer->Add(appellation_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // drink window
      auto* lbl_drink_window = new wxStaticText(this, wxID_ANY, "Drink Window:");
      lbl_drink_window->SetFont(default_font);
      details_sizer->Add(lbl_drink_window, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT|wxBOTTOM));
      auto* drink_window_val = new wxStaticText(this, wxID_ANY, "");
      drink_window_val->SetFont(default_font);
      drink_window_val->SetValidator(wxGenericValidator{ &m_details.drink_window });
      details_sizer->Add(drink_window_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM));

      // Scores heading
      auto* lbl_scores_header = new wxStaticText(this, wxID_ANY, "Scores");
      lbl_scores_header->SetFont(title_font);
      details_sizer->Add(lbl_scores_header, wxSizerFlags{}.Border(wxALL));
      details_sizer->AddSpacer(0);

      // My Score
      auto* lbl_my_score = new wxStaticText(this, wxID_ANY, "My Score:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_my_score->SetFont(default_font);
      details_sizer->Add(lbl_my_score, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* my_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_score_val->SetFont(default_font);
      my_score_val->SetValidator(wxGenericValidator(&m_details.my_score));
      details_sizer->Add(my_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // CT Score
      auto* lbl_ct_score = new wxStaticText(this, wxID_ANY, "CT Score:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_ct_score->SetFont(default_font);
      details_sizer->Add(lbl_ct_score, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* ct_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_score_val->SetFont(default_font);
      ct_score_val->SetValidator(wxGenericValidator{ &m_details.ct_score });
      details_sizer->Add(ct_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // Valuation heading
      auto* lbl_value_header = new wxStaticText(this, wxID_ANY, "Valuation");
      lbl_value_header->SetFont(title_font);
      details_sizer->Add(lbl_value_header, wxSizerFlags{}.Border(wxALL));
      details_sizer->AddSpacer(0);

      // My Price
      auto* lbl_my_price = new wxStaticText(this, wxID_ANY, "My Price:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_my_price->SetFont(default_font);
      details_sizer->Add(lbl_my_price, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* my_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_price_val->SetFont(default_font);
      my_price_val->SetMinSize(currency_min_size);
      my_price_val->SetMaxSize(currency_max_size);
      my_price_val->SetValidator(wxGenericValidator{ &m_details.my_price });
      details_sizer->Add(my_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // Community Avg
      auto* lbl_ct_price = new wxStaticText(this, wxID_ANY, "Community Avg:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_ct_price->SetFont(default_font);
      details_sizer->Add(lbl_ct_price, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* ct_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_price_val->SetFont(default_font);
      ct_price_val->SetMinSize(currency_min_size);
      ct_price_val->SetMaxSize(currency_max_size);
      ct_price_val->SetValidator(wxGenericValidator{ &m_details.community_price });
      details_sizer->Add(ct_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      // Auction value
      auto* lbl_auction_value = new wxStaticText(this, wxID_ANY, "Auction Value:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      lbl_auction_value->SetFont(default_font);
      details_sizer->Add(lbl_auction_value, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));
      auto* auction_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      auction_price_val->SetFont(default_font);
      auction_price_val->SetMinSize(currency_min_size);
      auction_price_val->SetMaxSize(currency_max_size);
      auction_price_val->SetValidator(wxGenericValidator{ &m_details.auction_value });
      details_sizer->Add(auction_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT|wxBOTTOM, border_size));

      top_sizer->Add(details_sizer, wxSizerFlags{1}.Expand().FixedMinSize().Border(wxALL));
      SetSizerAndFit(top_sizer);
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
      
      wxWindowUpdateLocker noUpdates(this);

      if (event.m_affected_row and event.m_affected_row.value() >= 0)
      {
         auto* tbl = event.m_grid_table;
         auto row_idx = event.m_affected_row.value();
         m_details.wine_name   = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_WINE_NAME  ).asString();
         m_details.vintage     = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_VINTAGE    ).asString();
         m_details.country     = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_COUNTRY    ).asString();
         m_details.region      = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_REGION     ).asString();
         m_details.sub_region  = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_SUB_REGION ).asString();
         m_details.appellation = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_APPELLATION).asString();

         m_details.drink_window     = detail::getDrinkWindow(tbl->getDetailProp(row_idx, constants::DETAIL_PROP_DRINK_START),
                                                             tbl->getDetailProp(row_idx, constants::DETAIL_PROP_DRINK_END));

         m_details.auction_value    = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_AUCTION_VALUE  ).asString(constants::FMT_NUMBER_CURRENCY).c_str();
         m_details.community_price  = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_COMMUNITY_PRICE).asString(constants::FMT_NUMBER_CURRENCY).c_str();
         m_details.my_price         = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_MY_PRICE       ).asString(constants::FMT_NUMBER_CURRENCY).c_str();

         m_details.ct_score         = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_CT_SCORE).asString(constants::FMT_NUMBER_DECIMAL).c_str();
         m_details.my_score         = tbl->getDetailProp(row_idx, constants::DETAIL_PROP_MY_SCORE).asString(constants::FMT_NUMBER_DECIMAL).c_str();
      }
      else{
         m_details = WineDetails{};
      }

      TransferDataToWindow();
      Layout();
      // TODO hide/unhide fields as necessary.
   }


} // namesapce ctb::app