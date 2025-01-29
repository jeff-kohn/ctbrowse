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

namespace ctb
{

   using namespace magic_enum;

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

      m_search_ctrl->SetFocus();
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
      m_menu_bar = new wxMenuBar();

      auto* menu_file = new wxMenu();
      auto* menu_sync_data = new wxMenuItem(menu_file, wxID_ANY, "&Sync Data...", "Download data from CellarTracker", wxITEM_NORMAL);
      menu_file->Append(menu_sync_data);

      menu_file->AppendSeparator();

      auto* menu_preferences = new wxMenuItem(menu_file, wxID_ANY, "&Preferences", "Configure App Preferences", wxITEM_NORMAL);
      menu_file->Append(menu_preferences);

      menu_file->AppendSeparator();

      auto* menu_quit = new wxMenuItem(menu_file, wxID_EXIT);
      menu_quit->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_QUIT, wxART_MENU));
      menu_file->Append(menu_quit);

      m_menu_bar->Append(menu_file, wxGetStockLabel(wxID_FILE));

      auto* menu = new wxMenu();
      auto* menu_item2 = new wxMenuItem(menu, wxID_FIND);
      menu_item2->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FIND, wxART_MENU));
      menu->Append(menu_item2);

      m_menu_bar->Append(menu, "&Edit");

      auto* menu_views = new wxMenu();
      auto* menu_item = new wxMenuItem(menu_views, wxID_ANY, "&Wine List\tCtrl+W");
      menu_views->Append(menu_item);

      m_menu_bar->Append(menu_views, "&View");

      SetMenuBar(m_menu_bar);

      // Event handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuPreferences, this, menu_preferences->GetId());
      Bind(wxEVT_MENU, &MainFrame::onMenuSyncData, this, menu_sync_data->GetId());
      Bind(wxEVT_MENU, &MainFrame::onMenuWineList, this, menu_item->GetId());
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

      m_search_ctrl = new wxSearchCtrl(m_tool_bar, wxID_ANY, wxEmptyString);
      m_search_ctrl->ShowSearchButton(true);
      m_search_ctrl->ShowCancelButton(true);
      m_tool_bar->AddControl(m_search_ctrl);

      m_tool_bar->Realize();

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
      m_search_ctrl->ChangeValue("");
      m_grid->clearSubStringFilter();
      updateStatusBarCounts();
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

} // namespace ctb
