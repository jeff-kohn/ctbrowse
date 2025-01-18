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


#include "cts/constants.h"
#include "cts/CredentialWrapper.h"

#include "cts/HttpStatusCodes.h"
#include "cts/winapi_util.h"

#include <wx/msgdlg.h>
#include <wx/progdlg.h>

namespace cts
{
   using namespace magic_enum;

   MainFrame::MainFrame() : m_data_mgr{ wxGetApp().userDataFolder() }
   {
   }

   MainFrame::MainFrame(wxWindow* parent)  : m_data_mgr{ wxGetApp().userDataFolder() }
   {
      Create(parent);
   }

   bool MainFrame::Create(wxWindow* parent)
   {
      // give base class a chance set up controls etc
      if (!MainFrameBase::Create(parent))
         return false;

      SetTitle(constants::APP_NAME_LONG);

      m_grid = new wxGrid(this, wxID_ANY);
      m_grid->EnableEditing(false);
      m_grid->EnableDragGridSize(false);
      m_grid->SetMargins(0, 0);
      m_grid->SetLabelBackgroundColour(wxColour("#FFFFFF"));
      m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
      m_grid->SetRowLabelSize(0);
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
      using namespace cts::data;

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

      CellarTrackerDownload::ProgressCallback progress_callback = [&progress_dlg] (int64_t downloadTotal, int64_t downloadNow,
                                                                                   int64_t uploadTotal, int64_t uploadNow,
                                                                                   intptr_t userdata)
                                                                  {
                                                                     return progress_dlg.Pulse();
                                                                  };

      // For each selected table, download it.
      for (auto tbl : dlg.selectedTables())
      {
         setStatusText(constants::FMT_STATUS_FILE_DOWNLOADING, CellarTrackerDownload::tableDescription(tbl));

         CellarTrackerDownload::DownloadResult result{};
         result = CellarTrackerDownload::getTableData(cred, tbl, DataFormatId::csv, &progress_callback);

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

         setStatusText(constants::FMT_STATUS_FILE_DOWNLOADED, CellarTrackerDownload::tableDescription(tbl));
      }

   }

   void MainFrame::onMenuWineList(wxCommandEvent&)
   {
      wxBusyCursor busy{};

      auto wine_list = std::make_unique<data::CtGridTable>(m_data_mgr.getWineList());
      m_grid->SetTable(wine_list.release(), true);
      m_grid->Refresh();
   }

   void MainFrame::onQuit([[maybe_unused]] wxCommandEvent& event)
   {
      Close(true);
   }




} // namespace cts
