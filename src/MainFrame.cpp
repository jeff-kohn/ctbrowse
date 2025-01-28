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
      if (!wxFrame::Create(parent, wxID_ANY, ""))
         return false;

      SetTitle(constants::APP_NAME_LONG);

      m_splitter = new wxSplitterWindow{ this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D };
      m_grid = new CellarTrackerGrid{ m_splitter };
      m_grid_tools_panel = new GridToolsPanel{ m_splitter, m_grid };
      createImpl();
      m_grid->ForceRefresh();
      Centre(wxBOTH);

      return true;
   }

   
   void MainFrame::onMenuPreferences(wxCommandEvent& event) 
   {

   }


   void MainFrame::onMenuSyncData(wxCommandEvent& event) 
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


   void MainFrame::onMenuWineList(wxCommandEvent&)
   {
      wxBusyCursor busy{};
      try
      {
         auto tbl = wxGetApp().getGridTable(GridTableMgr::GridTableId::WineList);
         assert(tbl);
         assert(m_grid);

         m_grid->setGridTable(tbl);
         Update();
      }
      catch(Error& e)
      {
         wxGetApp().displayErrorMessage(e);
      }
   }


   void MainFrame::onQuit([[maybe_unused]] wxCommandEvent& event)
   {
      Close(true);
   }

   
   void MainFrame::createImpl()
   {
      m_tool_bar = CreateToolBar();
      m_tool_bar->Realize();

      m_menu_bar = new wxMenuBar();

      auto* menu_file = new wxMenu();
      auto* menu_sync_data = new wxMenuItem(menu_file, wxID_ANY, "&Sync Data...",
         "Download data from CellarTracker", wxITEM_NORMAL);
      menu_file->Append(menu_sync_data);
      menu_file->AppendSeparator();
      auto* menu_preferences = new wxMenuItem(menu_file, wxID_ANY, "&Preferences", "Configure App Preferences",
         wxITEM_NORMAL);
      menu_file->Append(menu_preferences);
      menu_file->AppendSeparator();
      auto* menu_quit = new wxMenuItem(menu_file, wxID_EXIT);
      menu_quit->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_QUIT, wxART_MENU));

      menu_file->Append(menu_quit);
      m_menu_bar->Append(menu_file, wxGetStockLabel(wxID_FILE));

      auto* menu_views = new wxMenu();
      auto* menu_item = new wxMenuItem(menu_views, wxID_ANY, "&Wine List\tCTRL+W");
      menu_views->Append(menu_item);
      m_menu_bar->Append(menu_views, "&Data");

      SetMenuBar(m_menu_bar);

      m_status_bar = CreateStatusBar();

      m_splitter->SetSashGravity(0.0);
      m_splitter->SetMinimumPaneSize(150);
      m_splitter->SplitVertically(m_grid_tools_panel, m_grid);
      m_splitter->SetSashPosition(50);

      Centre(wxBOTH);

      Centre(wxBOTH);


      // Event handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuPreferences, this, menu_preferences->GetId());
      Bind(wxEVT_MENU, &MainFrame::onMenuSyncData, this, menu_sync_data->GetId());
      Bind(wxEVT_MENU, &MainFrame::onMenuWineList, this, menu_item->GetId());
   }

} // namespace ctb
