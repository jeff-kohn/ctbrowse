/*********************************************************************
 * @file       MainFrame.cpp
 *
 * @brief      Implementation for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "MainFrame.h"
#include "App.h"
#include "TableSyncDialog.h"
#include "wx_helpers.h"
#include "grids/GridTableWineList.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/data/table_download.h"
#include "ctb/winapi_util.h"
#include "external/HttpStatusCodes.h"

#include <wx/msgdlg.h>
#include <wx/progdlg.h>


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
      if (!MainFrameBase::Create(parent))
         return false;

      SetTitle(constants::APP_NAME_LONG);

      m_grid = new CellarTrackerGrid{ this };
      m_grid->Refresh();

      Bind(wxEVT_MENU, &MainFrame::onQuit, this, wxID_EXIT);

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
               wxMessageBox(result.error().error_message, constants::ERROR_STR, wxICON_ERROR | wxOK);
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
         auto tbl_ptr = std::dynamic_pointer_cast<GridTableWineList>(wxGetApp().getGridTable(GridTableMgr::GridTableId::WineList));
         assert(tbl_ptr);
         assert(m_grid);

         m_grid->setGridTable(tbl_ptr);
         Update();
      }
      catch(Error& e)
      {
         wxMessageBox(e.error_message, constants::ERROR_STR, wxICON_ERROR | wxOK);
      }
   }

   void MainFrame::onQuit([[maybe_unused]] wxCommandEvent& event)
   {
      Close(true);
   }




} // namespace ctb
