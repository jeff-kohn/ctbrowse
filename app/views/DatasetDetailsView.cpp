/*********************************************************************
 * @file       DatasetDetailsView.cpp
 *
 * @brief      implementation for the DatasetDetailsView class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "DatasetDetailsView.h"
#include "controls/WineDetailMainPanel.h"
#include "controls/WineDetailPendingPanel.h"
#include "controls/WineDetailScorePanel.h"
#include "controls/WineDetailTastingPanel.h"
#include "controls/WineDetailValuePanel.h"

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
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/wupdlock.h>


namespace ctb::constants
{
   constexpr auto LABEL_TIMER_RETRY_INTERVAL = 33; 

} // namespace ctb::constants


namespace ctb::app
{

   DatasetDetailsView::DatasetDetailsView(DatasetEventSourcePtr source, LabelCachePtr cache) : 
      m_event_sink{ this, std::move(source) },
      m_label_cache{ std::move(cache) }
   {}


   DatasetDetailsView* DatasetDetailsView::create(wxWindow* parent, const DatasetEventSourcePtr& source, LabelCachePtr cache)
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

      std::unique_ptr<DatasetDetailsView> wnd{ new DatasetDetailsView{ source, cache } };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release();
   }


   void DatasetDetailsView::initControls()
   {
      using enum CategorizedControls::Category;

      const     auto sizer_flags      = wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT );
      constexpr auto section_spacer   = 3;

      wxWindowUpdateLocker freeze_win(this);

      SetMinSize(ConvertDialogToPixels( wxSize{100, -1} ));
      auto* top_sizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(top_sizer);

      top_sizer->Add(new WineDetailMainPanel    { this, m_event_sink.getSource() }, sizer_flags);
      top_sizer->AddSpacer(section_spacer);
      top_sizer->Add(new WineDetailScorePanel   { this, m_event_sink.getSource() }, sizer_flags);
      top_sizer->AddSpacer(section_spacer);
      top_sizer->Add(new WineDetailValuePanel   { this, m_event_sink.getSource() }, sizer_flags);
      top_sizer->AddSpacer(section_spacer);
      top_sizer->Add(new WineDetailPendingPanel { this, m_event_sink.getSource() }, sizer_flags);
      top_sizer->AddSpacer(section_spacer);
      top_sizer->Add(new WineDetailTastingPanel { this, m_event_sink.getSource() }, sizer_flags);

      //// Command-Link buttons (Collection-Specific)
      addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_WINE_DETAILS,   LinkOpenWineDetails, constants::DETAILS_CMD_LINK_WINE_DETAILS);
      addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_ACCEPT_PENDING, LinkAcceptPending,   constants::DETAILS_CMD_LINK_ACCEPT_PENDING);
      addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_DRINK_REMOVE,   LinkReadyToDrink,    constants::DETAILS_CMD_LINK_DRINK_REMOVE);

      // Label image - won't correctly scale/redraw unless we use wxFULL_REPAINT_ON_RESIZE
      m_label_image = new wxGenericStaticBitmap(this, wxID_ANY, wxNullBitmap , wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
      top_sizer->Add(m_label_image, wxSizerFlags().CenterHorizontal().Expand().Shaped().Border(wxALL));
      m_label_image->SetScaleMode(wxStaticBitmap::Scale_AspectFit);
      m_category_controls.addControlDependency(BottleImage, m_label_image);
      
      top_sizer->ShowItems(false);

      // hook up event handlers
      m_label_timer.Bind(wxEVT_TIMER, &DatasetDetailsView::onLabelTimer, this);
   }


   void DatasetDetailsView::addCommandLinkButton(wxBoxSizer* sizer, CmdId cmd, CategorizedControls::Category category, std::string_view command_text, std::string_view note)
   {
      auto* link_button = new wxCommandLinkButton{ this, cmd, wxFromSV(command_text), wxFromSV(note), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER };
      sizer->Add(link_button, wxSizerFlags().Center().Border(wxLEFT|wxRIGHT));
      link_button->Bind(wxEVT_BUTTON, &DatasetDetailsView::onCommand, this, cmd);

      m_category_controls.addControlDependency(category, link_button);
   }


   void DatasetDetailsView::checkLabelResult()
   {
      using namespace tasks;

      if (auto& result = m_image_result)
      {
         switch (result->poll(0ms))
         {
            case wxImageTask::Status::Deferred: [[fallthrough]];
            case wxImageTask::Status::Finished:
               displayLabel();
               [[fallthrough]];

            case wxImageTask::Status::Invalid:
               m_image_result = {};
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


   void DatasetDetailsView::displayLabel()
   {
      try
      {
         if (m_image_result)
         {
            auto result = m_image_result->getImage();
            if (!result)
               throw Error{ std::move(result.error()) };

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


   void DatasetDetailsView::notify(DatasetEvent event)
   {
      try
      {
         updateDetails(event);   
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void DatasetDetailsView::updateDetails(DatasetEvent event)
   {
      using enum CategorizedControls::Category;

      wxWindowUpdateLocker freeze_win(this);
      
      if (event.affected_row.has_value())
      {
         GetSizer()->ShowItems(true);

         auto table_id = event.dataset->getTableId();
         m_category_controls.showCategory(BottleImage, true);
         m_category_controls.showCategory(LinkOpenWineDetails, table_id != TableId::Pending and table_id != TableId::Availability);
         m_category_controls.showCategory(LinkAcceptPending,   table_id == TableId::Pending);
         m_category_controls.showCategory(LinkReadyToDrink,    table_id == TableId::Availability);

         // image ctrl always starts hidden until background image fetch completes. But if it's already hidden, that 
         // means it's not used for this collection so leave it be and don't fetch background image
         if (m_label_image->IsShownOnScreen())
         {
            auto wine_id = event.dataset->getProperty(event.affected_row.value(), CtProp::iWineId).asUInt64().value_or(0);
            m_image_result = m_label_cache->fetchLabelImage(wine_id);
            m_label_image->Hide();
            checkLabelResult();
         }
      }
      else {
         GetSizer()->ShowItems(false);
      }

      // Need to force top-level sizer to re-layout since child panels may have changed size
      PostSizeEvent();
   }


   void DatasetDetailsView::onLabelTimer(wxTimerEvent&)
   {
      checkLabelResult();
   }


   void DatasetDetailsView::onCommand(wxCommandEvent& event)
   {
      wxQueueEvent(wxGetApp().GetTopWindow(), new wxCommandEvent{ wxEVT_MENU, event.GetId()});
   }


} // namespace ctb::app