/*********************************************************************
 * @file       DetailsViewBase.cpp
 *
 * @brief      implementation for the DetailsViewBase class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "DetailsViewBase.h"
#include "controls/WineDetailMainPanel.h"
#include "controls/WineDetailPendingPanel.h"
#include "controls/WineDetailScorePanel.h"
#include "controls/WineDetailTagsPanel.h"
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




namespace ctb::app
{

   void DetailsViewBase::createWindow(wxWindow* parent)
   {
      const auto sizer_flags = wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT);

      if (!this->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);

      // set up the sizer that all the detail panels will go into
      SetMinSize(ConvertDialogToPixels(wxSize{ 100, -1 }));
      auto* top_sizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(top_sizer);

      // add the base detail panel, then give derived classes the chance to add additional panels/fields.
      top_sizer->Add(WineDetailMainPanel::create(this, m_event_handler.getSource()), sizer_flags);
      this->addDatasetSpecificControls(top_sizer, m_event_handler.getSource());
   }


   void DetailsViewBase::addCommandLinkButton(wxBoxSizer* sizer, CmdId cmd)
   {
      static auto commands = std::map<CmdId, std::string_view>
      {
         { CmdId::CMD_ONLINE_WINE_DETAILS,      constants::DETAILS_CMD_LINK_WINE_DETAILS   },
         { CmdId::CMD_ONLINE_ACCEPT_PENDING,    constants::DETAILS_CMD_LINK_ACCEPT_PENDING },
         { CmdId::CMD_ONLINE_DRINK_REMOVE,      constants::DETAILS_CMD_LINK_DRINK_REMOVE   },
      };
      
      auto cmd_text = wxFromSV(commands[cmd]);
      if (cmd_text.empty())
      {
         assert("Unexpected CmdId passed to DetailsViewBase::addCommandLinkButton" and false);
         cmd_text = "???";
      }

      auto* link_button = new wxCommandLinkButton{ this, cmd, cmd_text, wxFromSV(constants::DETAILS_CMD_LINK_NOTE), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER};
      sizer->Add(link_button, wxSizerFlags().Center().Border(wxLEFT | wxRIGHT));
      link_button->Bind(wxEVT_BUTTON, &DetailsViewBase::onCommand, this, cmd);
   }

   void DetailsViewBase::onCommand(wxCommandEvent& event)
   {
      wxQueueEvent(wxGetApp().GetTopWindow(), new wxCommandEvent{ wxEVT_MENU, event.GetId() });
   }


} // namespace ctb::app