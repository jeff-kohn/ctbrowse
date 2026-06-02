/*********************************************************************
 * @file       DetailsViewBase.cpp
 *
 * @brief      implementation for the DetailsViewBase class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "DetailsViewBase.h"
#include "panels/WineDetailMainPanel.h"
#include "panels/WineDetailPendingPanel.h"
#include "panels/WineDetailScorePanel.h"
#include "panels/WineDetailTagsPanel.h"
#include "panels/WineDetailTastingPanel.h"
#include "panels/WineDetailValuePanel.h"

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
      SetMinSize(ConvertDialogToPixels(wxSize{ constants::pix_100, -1 }));
      auto* top_sizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(top_sizer);

      // add the base detail panel, then give derived classes the chance to add additional panels/fields.
      top_sizer->Add(WineDetailMainPanel::create(this, getEventHandler().getSource()), sizer_flags);
      this->addDatasetSpecificControls(top_sizer, getEventHandler().getSource());

      // As the selected wine changes, the contents of our subpanels may dynamically adjust size/contents, we we need to re-layout the window and
      // sending a WM_SIZE event is the most reliable way to do that. Using CallAfter() ensures that this will happen after all other subscribers
      // have handled the dataset event.
      getEventHandler().addHandler(DatasetEvent::Id::RowSelected, [this](auto&&) { CallAfter([this] { SendSizeEvent(); }); });
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

   // NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
   void DetailsViewBase::onCommand(wxCommandEvent& event)
   {
      wxQueueEvent(wxGetApp().GetTopWindow(), new wxCommandEvent{ wxEVT_MENU, event.GetId() }); 
   }
   // NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)


} // namespace ctb::app
