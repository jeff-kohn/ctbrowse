#include "panels/WineDetailsPanel.h"

// order dependent includes
#include <wx/stattext.h>
#include <wx/generic/stattextg.h>

#include <wx/valgen.h>

namespace ctb::app
{

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
      auto* top_sizer = new wxBoxSizer(wxVERTICAL);

      auto* details_sizer = new wxFlexGridSizer(2, 0, 0);
      details_sizer->SetMinSize(ConvertDialogToPixels(wxSize{ 75, constants::WX_UNSPECIFIED_VALUE }));

      auto* lbl_wine_name = new wxGenericStaticText(this, wxID_ANY, wxEmptyString);
      lbl_wine_name->SetLabelMarkup("<b>Wine:</b>");
      details_sizer->Add(lbl_wine_name, wxSizerFlags().Border(wxALL));

      m_wine_name_link = new wxGenericHyperlinkCtrl(this, wxID_ANY, "Wine Name", "http://cellartracker.com");
      m_wine_name_link->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
      details_sizer->Add(m_wine_name_link, wxSizerFlags().Border(wxALL));

      auto* lbl_country_region = new wxStaticText(this, wxID_ANY, "Country/Region:");
      details_sizer->Add(lbl_country_region, wxSizerFlags().Border(wxALL));

      m_country_region_txt = new wxStaticText(this, wxID_ANY, "country and region");
      details_sizer->Add(m_country_region_txt, wxSizerFlags().Border(wxALL));

      auto* lbl_appellation = new wxStaticText(this, wxID_ANY, "Appellation:");
      details_sizer->Add(lbl_appellation, wxSizerFlags().Border(wxALL));

      m_appellation_txt = new wxStaticText(this, wxID_ANY, "Appellation");
      details_sizer->Add(m_appellation_txt, wxSizerFlags().Border(wxALL));

      auto* lbl_drink_window = new wxStaticText(this, wxID_ANY, "Drink Window:");
      details_sizer->Add(lbl_drink_window, wxSizerFlags().Border(wxALL));

      m_drink_window_link = new wxGenericHyperlinkCtrl(this, wxID_ANY, "Drink By", "http//cellartracker.com",
         wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_RIGHT);
      m_drink_window_link->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
      details_sizer->Add(m_drink_window_link, wxSizerFlags().Border(wxALL));

      top_sizer->Add(details_sizer, wxSizerFlags().Expand().FixedMinSize().Border(wxALL));

      m_score_pane = new wxCollapsiblePane(this, wxID_ANY, "Scores");
      m_score_pane->Expand();
      top_sizer->Add(m_score_pane,
         wxSizerFlags().Border(wxLEFT|wxRIGHT|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

      auto* scores_sizer = new wxGridSizer(2, 0, 0);

      auto* lbl_my_score = new wxStaticText(m_score_pane->GetPane(), wxID_ANY, "My Score:");
      scores_sizer->Add(lbl_my_score, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      m_my_score_txt = new wxStaticText(m_score_pane->GetPane(), wxID_ANY, "98.1", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      //m_my_score_txt->SetValidator(wxGenericValidator(&m_my_score_val));
      scores_sizer->Add(m_my_score_txt, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      auto* lbl_ct_score = new wxStaticText(m_score_pane->GetPane(), wxID_ANY, "CT Score:");
      scores_sizer->Add(lbl_ct_score, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      m_ct_score_txt = new wxStaticText(m_score_pane->GetPane(), wxID_ANY, "93.2", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      scores_sizer->Add(m_ct_score_txt, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));
      m_score_pane->GetPane()->SetSizerAndFit(scores_sizer);

      m_value_pane = new wxCollapsiblePane(this, wxID_ANY, "Valuation");
      m_value_pane->Expand();
      top_sizer->Add(m_value_pane, wxSizerFlags().Border(wxALL));

      auto* value_sizer = new wxGridSizer(2, 0, 0);

      auto* lbl_my_price = new wxStaticText(m_value_pane->GetPane(), wxID_ANY, "My Price:");
      value_sizer->Add(lbl_my_price, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      m_my_price_txt = new wxStaticText(m_value_pane->GetPane(), wxID_ANY, "$100.00", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      value_sizer->Add(m_my_price_txt, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      auto* lbl_ct_price = new wxStaticText(m_value_pane->GetPane(), wxID_ANY, "Community Avg:");
      value_sizer->Add(lbl_ct_price, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      m_ct_price_txt = new wxStaticText(m_value_pane->GetPane(), wxID_ANY, "$100.00", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      value_sizer->Add(m_ct_price_txt, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      auto* lbl_auction_value = new wxStaticText(m_value_pane->GetPane(), wxID_ANY, "Auction Value:");
      value_sizer->Add(lbl_auction_value, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

      m_auction_price_txt = new wxStaticText(m_value_pane->GetPane(), wxID_ANY, "$100.00", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
      value_sizer->Add(m_auction_price_txt, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));
      m_value_pane->GetPane()->SetSizerAndFit(value_sizer);

      SetSizerAndFit(top_sizer);
      SendSizeEventToParent();

      // event handlers
      m_score_pane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &WineDetailsPanel::onPaneOpenClose, this);

   }


   void WineDetailsPanel::notify(GridTableEvent event, [[maybe_unused]] GridTable* grid_table)
   {
      try
      {
         switch (event)
         {
         case GridTableEvent::TableInitialize:
            // TODO
            break;

         case GridTableEvent::Sort:
            // TODO
            break;

         case GridTableEvent::Filter:
         case GridTableEvent::SubStringFilter:
         case GridTableEvent::RowSelected:
         default:
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

   void WineDetailsPanel::onPaneOpenClose(wxCollapsiblePaneEvent& event)
   {
      //Layout();
      //Fit()
      //this->SendSizeEvent();
   }

} // namesapce ctb::app