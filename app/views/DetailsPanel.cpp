/*********************************************************************
 * @file       DetailsPanel.cpp
 *
 * @brief      implementation for the DetailsPanel class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "views/DetailsPanel.h"

#include <ctb/utility_chrono.h>
#include <ctb/utility_http.h>
#include <ctb/tasks/tasks.h>

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
      auto getDrinkWindow(const CtPropertyVal& drink_start, const CtPropertyVal& drink_end) -> wxString
      {
         if (drink_start.isNull() && drink_end.isNull())
            return wxEmptyString;

         if (drink_start.isNull() && !drink_end.isNull())
            return drink_end.asString("By {}").c_str();

         if (drink_end.isNull())
            return drink_start.asString("{}+").c_str();

         return ctb::format("{} - {}", drink_start.asString(), drink_end.asString());
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
      auto heading_font{ GetFont().MakeBold() };
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
      static constexpr int cols = 2;
      auto* details_sizer = new wxFlexGridSizer(cols, 0, 0);

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
      drink_window_lbl->SetValidator(wxGenericValidator{ &m_drink_window_label });
      details_sizer->Add(drink_window_lbl, wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* drink_window_val = new wxStaticText(this, wxID_ANY, "");
      drink_window_val->SetValidator(wxGenericValidator{ &m_details.drink_window });
      details_sizer->Add(drink_window_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));
      m_category_controls.addControlDependency(DrinkWindow, drink_window_lbl);
      m_category_controls.addControlDependency(DrinkWindow, drink_window_val);

      // CT drink window (only for Availability view)
      auto* ct_drink_window_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_DRINK_WINDOW_CT);
      details_sizer->Add(ct_drink_window_lbl , wxSizerFlags{}.Right().Border(wxLEFT|wxRIGHT));
      auto* ct_drink_window_val = new wxStaticText(this, wxID_ANY, "");
      ct_drink_window_val->SetValidator(wxGenericValidator{ &m_details.ct_drink_window });
      details_sizer->Add(ct_drink_window_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT));
      m_category_controls.addControlDependency(CtDrinkWindow, ct_drink_window_lbl );
      m_category_controls.addControlDependency(CtDrinkWindow, ct_drink_window_val);

      // Scores heading
      auto* scores_header_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_SCORES, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      scores_header_lbl->SetFont(heading_font);
      scores_header_lbl->SetForegroundColour(heading_color);
      details_sizer->Add(scores_header_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxTOP, border_size));
      details_sizer->AddSpacer(0);
      m_category_controls.addControlDependency(Score, scores_header_lbl);

      // My Score
      auto* my_score_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_MY_SCORE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(my_score_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* my_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_score_val->SetValidator(wxGenericValidator(&m_details.my_score));
      details_sizer->Add(my_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Score, my_score_lbl);
      m_category_controls.addControlDependency(Score, my_score_val);

      // CT Score
      auto* ct_score_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_CT_SCORE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(ct_score_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* ct_score_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_score_val->SetValidator(wxGenericValidator{ &m_details.ct_score });
      details_sizer->Add(ct_score_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Score, ct_score_lbl);
      m_category_controls.addControlDependency(Score, ct_score_val);

      // Valuation heading
      auto* value_header_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_VALUATION, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      value_header_lbl->SetFont(heading_font);
      value_header_lbl->SetForegroundColour(heading_color);
      details_sizer->Add(value_header_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxTOP, border_size));
      details_sizer->AddSpacer(0);
      m_category_controls.addControlDependency(Valuation, value_header_lbl);

      // My Price
      auto* my_price_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_MY_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(my_price_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* my_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      my_price_val->SetValidator(wxGenericValidator{ &m_details.my_price });
      details_sizer->Add(my_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Valuation, my_price_lbl);
      m_category_controls.addControlDependency(Valuation, my_price_val);

      // Community Avg
      auto* ct_price_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_CT_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(ct_price_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* ct_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      ct_price_val->SetValidator(wxGenericValidator{ &m_details.community_price });
      details_sizer->Add(ct_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Valuation, ct_price_lbl);
      m_category_controls.addControlDependency(Valuation, ct_price_val);

      // Auction value
      auto* auction_value_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_AUCTION_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(auction_value_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* auction_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      auction_price_val->SetValidator(wxGenericValidator{ &m_details.auction_value });
      details_sizer->Add(auction_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Valuation, auction_value_lbl);
      m_category_controls.addControlDependency(Valuation, auction_value_lbl);

      // Pending Order details heading
      auto* order_details_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_ORDER_DETAILS, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      order_details_lbl->SetFont(heading_font);
      order_details_lbl->SetForegroundColour(heading_color);
      details_sizer->Add(order_details_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxTOP, border_size));
      details_sizer->AddSpacer(0);
      m_category_controls.addControlDependency(Pending, order_details_lbl);

      // pending store name
      auto* pend_store_name_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_STORE_NAME, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(pend_store_name_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* pend_store_name_val = new wxStaticText(this, wxID_ANY, "");
      pend_store_name_val->SetValidator(wxGenericValidator{ &m_details.pending_store_name });
      details_sizer->Add(pend_store_name_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Pending, pend_store_name_lbl);
      m_category_controls.addControlDependency(Pending, pend_store_name_val);

      // pending quantity
      auto* pend_order_qty_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_QTY_ORDERED, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(pend_order_qty_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* pend_order_qty_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      pend_order_qty_val->SetValidator(wxGenericValidator{ &m_details.pending_qty });
      details_sizer->Add(pend_order_qty_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Pending, pend_order_qty_lbl);
      m_category_controls.addControlDependency(Pending, pend_order_qty_val);  

      // pending price
      auto* pending_price_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_MY_PRICE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(pending_price_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* pending_price_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      pending_price_val->SetValidator(wxGenericValidator{ &m_details.pending_price });
      details_sizer->Add(pending_price_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Pending, pending_price_lbl);
      m_category_controls.addControlDependency(Pending, pending_price_val);  

      // pending order date
      auto* pend_order_date_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_ORDER_DATE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(pend_order_date_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* pend_order_date_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      pend_order_date_val->SetValidator(wxGenericValidator{ &m_details.pending_order_date });
      details_sizer->Add(pend_order_date_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Pending, pend_order_date_lbl);
      m_category_controls.addControlDependency(Pending, pend_order_date_val);

      // pending delivery date
      auto* pend_delivery_date_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_DELIVERY_DATE, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(pend_delivery_date_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* pend_delivery_date_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      pend_delivery_date_val->SetValidator(wxGenericValidator{ &m_details.pending_delivery_date });
      details_sizer->Add(pend_delivery_date_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Pending, pend_delivery_date_lbl);
      m_category_controls.addControlDependency(Pending, pend_delivery_date_val);

      // order #
      auto* pend_order_num_lbl = new wxStaticText(this, wxID_ANY, constants::LBL_ORDER_NUMBER, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      details_sizer->Add(pend_order_num_lbl, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT, border_size));
      auto* pend_order_num_val = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      pend_order_num_val->SetValidator(wxGenericValidator{ &m_details.pending_order_number });
      details_sizer->Add(pend_order_num_val, wxSizerFlags{}.Border(wxLEFT|wxRIGHT, border_size));
      m_category_controls.addControlDependency(Pending, pend_order_num_lbl);
      m_category_controls.addControlDependency(Pending, pend_order_num_val);

      // end details_sizer layout
      top_sizer->Add(details_sizer, wxSizerFlags{}.CenterHorizontal().FixedMinSize().Border(wxALL));

      // Command-Link buttons (Collection-Specific)
      addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_WINE_DETAILS, LinkOpenWineDetails, constants::DETAILS_CMD_LINK_WINE_DETAILS);
      addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_ACCEPT_PENDING, LinkAcceptPending,   constants::DETAILS_CMD_LINK_ACCEPT_PENDING);
      addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_DRINK_REMOVE,   LinkReadyToDrink,    constants::DETAILS_CMD_LINK_DRINK_REMOVE);

      // image won't correctly scale/redraw unless we use wxFULL_REPAINT_ON_RESIZE
      m_label_image = new wxGenericStaticBitmap(this, wxID_ANY, wxNullBitmap , wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
      m_label_image->SetScaleMode(wxStaticBitmap::Scale_AspectFit);
      top_sizer->Add(m_label_image, wxSizerFlags().CenterHorizontal().Expand().Shaped().Border(wxALL));

      SetSizerAndFit(top_sizer);
      top_sizer->ShowItems(false);

      // hook up event handlers
      m_label_timer.Bind(wxEVT_TIMER, &DetailsPanel::onLabelTimer, this);
   }

   void DetailsPanel::addCommandLinkButton(wxBoxSizer* sizer, CmdId cmd, CategorizedControls::Category category, std::string_view command_text, std::string_view note)
   {
      auto* link_button = new wxCommandLinkButton{ this, cmd, wxFromSV(command_text), wxFromSV(note), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER };
      sizer->Add(link_button, wxSizerFlags().CenterHorizontal().Border(wxALL));
      link_button->Bind(wxEVT_BUTTON, &DetailsPanel::onCommand, this, cmd);

      m_category_controls.addControlDependency(category, link_button);

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

         // note that we try to grab all properties even though some of them won't be available in this dataset,
         // but that's fine because we'll just get a null value if it's not available, so no need to check hasProperty()

         m_details.wine_id     = dataset->getProperty(rec_idx, CtProp::iWineId     ).asString();
         m_details.wine_name   = dataset->getProperty(rec_idx, CtProp::WineName    ).asString();
         m_details.vintage     = dataset->getProperty(rec_idx, CtProp::Vintage     ).asString();
         m_details.varietal    = dataset->getProperty(rec_idx, CtProp::Varietal    ).asString();
         m_details.country     = dataset->getProperty(rec_idx, CtProp::Country     ).asString();
         m_details.region      = dataset->getProperty(rec_idx, CtProp::Region      ).asString();
         m_details.sub_region  = dataset->getProperty(rec_idx, CtProp::SubRegion   ).asString();
         m_details.appellation = dataset->getProperty(rec_idx, CtProp::Appellation ).asString();

         m_details.drink_window     = detail::getDrinkWindow(dataset->getProperty(rec_idx, CtProp::BeginConsume ),
                                                             dataset->getProperty(rec_idx, CtProp::EndConsume   ));

         m_details.ct_drink_window  = detail::getDrinkWindow(dataset->getProperty(rec_idx, CtProp::CtBeginConsume ),
                                                             dataset->getProperty(rec_idx, CtProp::CtEndConsume   ));

         m_details.auction_value    = dataset->getProperty(rec_idx, CtProp::AuctionPrice ).asString(constants::FMT_NUMBER_CURRENCY);
         m_details.community_price  = dataset->getProperty(rec_idx, CtProp::CtPrice      ).asString(constants::FMT_NUMBER_CURRENCY);
         m_details.my_price         = dataset->getProperty(rec_idx, CtProp::MyPrice      ).asString(constants::FMT_NUMBER_CURRENCY);

         auto prop_val = dataset->getProperty(rec_idx, CtProp::CtScore);
         m_details.ct_score = prop_val ? prop_val.asString(constants::FMT_NUMBER_DECIMAL) : constants::NO_SCORE;

         prop_val = dataset->getProperty(rec_idx, CtProp::MyScore);
         m_details.my_score = prop_val ? prop_val.asString(constants::FMT_NUMBER_DECIMAL) : constants::NO_SCORE;

         m_details.pending_purchase_id   = dataset->getProperty(rec_idx, CtProp::PendingPurchaseId   ).asString();
         m_details.pending_order_number  = dataset->getProperty(rec_idx, CtProp::PendingOrderNumber  ).asString();
         m_details.pending_order_date    = dataset->getProperty(rec_idx, CtProp::PendingOrderDate    ).asString(constants::FMT_DATE_SHORT);
         m_details.pending_delivery_date = dataset->getProperty(rec_idx, CtProp::PendingDeliveryDate ).asString(constants::FMT_DATE_SHORT);
         m_details.pending_store_name    = dataset->getProperty(rec_idx, CtProp::PendingStoreName    ).asString();
         m_details.pending_qty           = dataset->getProperty(rec_idx, CtProp::PendingOrderQty     ).asString();
         m_details.pending_price         = dataset->getProperty(rec_idx, CtProp::PendingPrice        ).asString(constants::FMT_NUMBER_CURRENCY);

         // show everything since detail panel may be blank if no record was selected previously...
         GetSizer()->ShowItems(true); 

         // but show/hide control categories as appropriate.
         configureControlsForDataset(dataset);

         // image ctrl always starts hidden until background image fetch completes
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
      SendSizeEvent();
      Update();
   }


   void DetailsPanel::notify(DatasetEvent event)
   {
      try
      {
         // TODO: Re-org/clarify
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

      // Details display
      m_category_controls.showCategory(Score, dataset->hasProperty(CtProp::CtScore));
      m_category_controls.showCategory(DrinkWindow, dataset->hasProperty(CtProp::BeginConsume));
      m_category_controls.showCategory(CtDrinkWindow, dataset->hasProperty(CtProp::CtBeginConsume));
      m_category_controls.showCategory(Pending, dataset->hasProperty(CtProp::PendingPurchaseId));
      m_category_controls.showCategory(Valuation, dataset->hasProperty(CtProp::MyPrice));
      if (dataset->hasProperty(CtProp::CtBeginConsume))
      {
         m_drink_window_label = constants::LBL_DRINK_WINDOW_MY;
         TransferDataToWindow();
      }
      else {
         m_drink_window_label = constants::LBL_DRINK_WINDOW;
      }

      // Command-Link buttons
      m_category_controls.showCategory(LinkAcceptPending,   dataset->getTableId() == TableId::Pending);
      m_category_controls.showCategory(LinkOpenWineDetails, dataset->getTableId() == TableId::List);
      m_category_controls.showCategory(LinkReadyToDrink,    dataset->getTableId() == TableId::Availability);
   }


   void DetailsPanel::onLabelTimer(wxTimerEvent&)
   {
      checkLabelResult();
   }


   void DetailsPanel::onCommand(wxCommandEvent& event)
   {
      wxQueueEvent(wxGetApp().GetTopWindow(), new wxCommandEvent{ wxEVT_MENU, event.GetId()});
   }

} // namesapce ctb::app