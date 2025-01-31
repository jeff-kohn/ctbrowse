/*********************************************************************
 * @file       MainFrame.cpp
 *
 * @brief      Implementation for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "MainFrame.h"
#include "App.h"
#include "dialogs/TableSyncDialog.h"
#include "grids/GridTableWineList.h"
#include "wx_helpers.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/data/table_download.h"
#include "ctb/winapi_util.h"
#include "external/HttpStatusCodes.h"

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/stockitem.h>
#include <wx/xrc/xmlres.h>

namespace ctb::app
{

   using namespace magic_enum;

   // we don't use enum class because then every time we need to pass an ID to wxObject,
   // we'd have to case or use std::to_underlying and that's just an ugly waste of time for this.
   enum CmdId
   {
      ID_FILE_DOWNLOAD_DATA = wxID_HIGHEST,
      ID_FILE_SETTINGS,
      ID_VIEW_WINE_LIST
   };


   MainFrame::MainFrame()
   {
   }


   MainFrame::MainFrame(wxWindow* parent)   
   {
      Create(parent);
   }


   bool MainFrame::Create(wxWindow* parent)
   {
      // give base class a chance set up controls etc
      if (!wxFrame::Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize{640, 480}))
         return false;

      SetTitle(constants::APP_NAME_LONG);

      createGrid();
      createMenuBar();
      createStatusBar();
      createToolBar();

      Centre(wxBOTH);

      return true;
   }


   void MainFrame::createGrid()
   {
      m_grid = new CellarTrackerGrid{ this };
      m_grid->SetMinSize(ConvertDialogToPixels(wxSize(400, 250)));
      m_grid->SetMargins(0, 0);
      m_grid->SetColLabelSize(30);
   }


   void MainFrame::createMenuBar()
   {
      using namespace constants;

      m_menu_bar = new wxMenuBar();

      // File menu
      //
      auto* menu_file = new wxMenu();

      auto* menu_sync_data = new wxMenuItem{
         menu_file, 
         ID_FILE_DOWNLOAD_DATA, 
         constants::CMD_FILE_DOWNLOAD_DATA, 
         constants::CMD_FILE_DOWNLOAD_DATA_TIP, 
         wxITEM_NORMAL
      };
      menu_file->Append(menu_sync_data);
      menu_file->AppendSeparator();

      auto* menu_file_preferences = new wxMenuItem{
         menu_file, 
         ID_FILE_SETTINGS, 
         constants::CMD_FILE_SETTINGS,
         constants::CMD_FILE_SETTINGS_TIP,
         wxITEM_NORMAL
      };
      menu_file->Append(menu_file_preferences);
      menu_file->AppendSeparator();

      auto* menu_file_quit = new wxMenuItem(menu_file, wxID_EXIT);
      menu_file->Append(menu_file_quit);
      m_menu_bar->Append(menu_file, wxGetStockLabel(wxID_FILE));
   

      // Edit Menu
      //
      auto* menu_edit = new wxMenu();
      auto* menu_edit_find = new wxMenuItem(menu_edit, wxID_FIND);
      menu_edit_find->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FIND, wxART_MENU));
      menu_edit->Append(menu_edit_find);
      m_menu_bar->Append(menu_edit, wxGetStockLabel(wxID_EDIT));


      // Views Menu
      //
      auto* menu_views = new wxMenu();
      auto* menu_views_wine_list = new wxMenuItem{
         menu_views, 
         ID_VIEW_WINE_LIST, 
         constants::CMD_VIEWS_WINE_LIST, 
         constants::CMD_VIEWS_WINE_LIST_TIP,
         wxITEM_NORMAL
      };
      menu_views->Append(menu_views_wine_list);
      m_menu_bar->Append(menu_views, constants::MENU_VIEWS);

      SetMenuBar(m_menu_bar);

      // Event handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuPreferences, this, menu_file_preferences->GetId());
      Bind(wxEVT_MENU, &MainFrame::onMenuSyncData, this, menu_sync_data->GetId());
      Bind(wxEVT_MENU, &MainFrame::onMenuWineList, this, menu_views_wine_list->GetId());
      Bind(wxEVT_MENU, &MainFrame::onQuit, this, wxID_EXIT);
   }


   void MainFrame::createStatusBar()
   {
      m_status_bar = CreateStatusBar(3);
      const int sb_field_widths[3] = {-4, -1, -1};
      m_status_bar->SetStatusWidths(3, sb_field_widths);
      const int sb_field_styles[3] = {wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL};
      m_status_bar->SetStatusStyles(3, sb_field_styles);
   }


   void MainFrame::createToolBar()
   {
      m_tool_bar = CreateToolBar();

      auto bmp = wxBitmapBundle::FromSVGResource("TOOLBAR_SYNCHRONIZE", FromDIP(wxSize(24, 24)));

      m_tool_bar->AddTool(wxID_ANY, wxEmptyString, bmp);

      m_search_ctrl = new wxSearchCtrl(m_tool_bar, wxID_ANY, wxEmptyString);
      m_search_ctrl->ShowSearchButton(true);
      m_search_ctrl->ShowCancelButton(true);

      m_tool_bar->AddControl(m_search_ctrl);
      m_tool_bar->Realize();
      m_search_ctrl->SetFocus();

      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &MainFrame::onSearchCancelBtn, this);
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &MainFrame::onSearchBtn, this);
      m_search_ctrl->Bind(wxEVT_TEXT_ENTER, &MainFrame::onSearchTextEnter, this);

   }


   void MainFrame::onMenuEditFind([[maybe_unused]] wxCommandEvent& event)
   {
      m_tool_bar->SetFocus();
      m_search_ctrl->SetFocus();
   }


   void MainFrame::onMenuPreferences([[maybe_unused]] wxCommandEvent& event)
   {

   }


   void MainFrame::onMenuSyncData([[maybe_unused]] wxCommandEvent& event) 
   {
      using namespace ctb::data;

      TableSyncDialog dlg(this);
      if (dlg.ShowModal() != wxID_OK)
         return;

      wxBusyCursor busy{};
      ScopedStatusText end_status{ constants::STATUS_DOWNLOAD_COMPLETE, this };

      CredentialWrapper cred_wrapper{
         constants::CELLARTRACKER_DOT_COM, true,
         constants::CELLARTRACKER_LOGON_CAPTION,
         constants::CELLARTRACKER_LOGON_TITLE
      };

      auto cred_result = cred_wrapper.promptForCredential();
      if (!cred_result.has_value())
      {
         end_status.message = constants::STATUS_DOWNLOAD_CANCELED;
      }
      auto& cred = cred_result.value();

      wxProgressDialog progress_dlg{"Download Progress", "Downloading Data Files", 100, this, wxPD_CAN_ABORT | wxPD_AUTO_HIDE | wxPD_APP_MODAL };

      data::ProgressCallback progress_callback = [&progress_dlg] (int64_t downloadTotal, int64_t downloadNow,
                                                                  int64_t uploadTotal, int64_t uploadNow,
                                                                  intptr_t userdata)
                                                                  {
                                                                     return progress_dlg.Pulse();
                                                                  };

      // For each selected table, download it.
      for (auto tbl : dlg.selectedTables())
      {
         setStatusText(constants::FMT_STATUS_FILE_DOWNLOADING, data::getTableDescription(tbl));

         data::DownloadResult result{};
         result = downloadRawTableData(cred, tbl, DataFormatId::csv, &progress_callback);

         // we may need to re-prompt 
         while (!result.has_value() && result.error().error_code == enum_index(HttpStatus::Code::Unauthorized))
         {
            cred_result = cred_wrapper.promptForCredential();
            if (cred_result)
            {
               cred = cred_result.value();
            }
            else
            {
               end_status.message = constants::ERROR_DOWNLOAD_AUTH_FAILURE;
               return;
            }
         }

         if (!result)
         {
            // we didn't get a result, so indicate error to user.
            if (result.error().category == Error::Category::OperationCanceled)
            {
               end_status.message = constants::STATUS_DOWNLOAD_CANCELED;
            }
            else
            {
               wxGetApp().displayErrorMessage(result.error());
               end_status.message = constants::STATUS_DOWNLOAD_FAILED;
            }
            return;
         }

         // if we get here we have the data, so save it to file.
         auto folder = wxGetApp().userDataFolder();
         auto file_path{ folder / result->tableName() };
         file_path.replace_extension(constants::DATA_FILE_EXTENSION);
         util::saveTextToFile(result->data, file_path);

         setStatusText(constants::FMT_STATUS_FILE_DOWNLOADED, data::getTableDescription(tbl));
      }
   }


   void MainFrame::onMenuWineList([[maybe_unused]] wxCommandEvent& event)
   {
      wxBusyCursor busy{};
      try
      {
         auto tbl = wxGetApp().getGridTable(GridTableMgr::GridTableId::WineList);
         assert(tbl);
         assert(m_grid);

         m_grid->setGridTable(tbl);
         updateStatusBarCounts();
         Update();
      }
      catch(Error& e)
      {
         wxGetApp().displayErrorMessage(e);
      }
   }


   void MainFrame::onSearchBtn([[maybe_unused]] wxCommandEvent& event)
   {
      doSearchFilter();
   }


   void MainFrame::onSearchCancelBtn([[maybe_unused]] wxCommandEvent& event)
   {
      clearSearchFilter();
   }


   void MainFrame::onSearchTextEnter([[maybe_unused]] wxCommandEvent& event)
   {
      doSearchFilter();
   }


   void MainFrame::onQuit([[maybe_unused]] wxCommandEvent& event)
   {
      Close(true);
   }


   void MainFrame::doSearchFilter()
   {
      try
      {
         m_grid->filterBySubstring(m_search_ctrl->GetValue().wx_str());
         updateStatusBarCounts();
      }
      catch(Error& e)
      {
         wxGetApp().displayErrorMessage(e);
      }
   }


   void MainFrame::clearSearchFilter()
   {
      try
      {
         m_search_ctrl->ChangeValue("");
         m_grid->clearSubStringFilter();
         updateStatusBarCounts();
      }
      catch(Error& e)
      {
         wxGetApp().displayErrorMessage(e);
      }
   }


   void MainFrame::updateStatusBarCounts()
   {
      assert(m_grid);
      auto total = m_grid->getTotalRowCount();
      auto filtered = m_grid->getFilteredRowCount();

      SetStatusText(std::format(constants::FMT_LBL_TOTAL_ROWS, total), STATUS_BAR_PANE_TOTAL_ROWS);
      
      if (filtered < total)
         SetStatusText(std::format(constants::FMT_LBL_FILTERED_ROWS, filtered), STATUS_BAR_PANE_FILTERED_ROWS);
      else
         SetStatusText("", STATUS_BAR_PANE_FILTERED_ROWS);
   }

} // namespace ctb::app
