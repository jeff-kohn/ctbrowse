/*********************************************************************
 * @file       MainFrame.cpp
 *
 * @brief      Implementation for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "MainFrame.h"
#include "wx_helpers.h"
#include "dialogs/TableSyncDialog.h"
#include "grid/CellarTrackerGrid.h"
#include "grid/GridTableLoader.h"
#include "grid/GridTableWineList.h"
#include "panels/GridOptionsPanel.h"

#include <ctb/CredentialWrapper.h>
#include <ctb/data/table_download.h>
#include <ctb/winapi_util.h>
#include <external/HttpStatusCodes.h>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/grid.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/persist/toplevel.h>
#include <wx/progdlg.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/statusbr.h>
#include <wx/stockitem.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <memory>


namespace ctb::app
{

   using namespace magic_enum;

   // we don't use enum class because then every time we need to pass an ID to wxObject,
   // we'd have to cast or use std::to_underlying and that's just an ugly waste of time 
   // with no benefit for this use-case.
   enum CmdId
   {
      CMD_FILE_DOWNLOAD_DATA = wxID_HIGHEST,
      CMD_FILE_SETTINGS,
      CMD_VIEW_WINE_LIST
   };


   MainFrame::MainFrame() : m_event_source{ GridTableEventSource::create() }
   {
   }


   [[nodiscard]] MainFrame* MainFrame::create()
   {
      // give base class a chance set up controls etc
      std::unique_ptr<MainFrame> wnd{ new MainFrame{} };

      const auto default_size = wnd->FromDIP(wxSize{640, 480});
      if (!wnd->Create(nullptr, wxID_ANY, constants::APP_NAME_LONG, wxDefaultPosition, default_size))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release(); // top-level window manages its own lifetime, we return non-owning pointer
   }


   void MainFrame::initControls()
   {
      SetTitle(constants::APP_NAME_LONG);
      SetIcon(wxIcon{constants::RES_NAME_ICON_PRODUCT});
      SetName(constants::RES_NAME_MAINFRAME);               // needed for wxPersistence support

      // No createGridWindows() call here, we'll create it once we have some data 
      createMenuBar();
      createStatusBar();
      createToolBar();

      // Event handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuPreferences, this, CmdId::CMD_FILE_SETTINGS);
      Bind(wxEVT_MENU, &MainFrame::onMenuSyncData, this, CmdId::CMD_FILE_DOWNLOAD_DATA);
      Bind(wxEVT_MENU, &MainFrame::onMenuWineList, this, CmdId::CMD_VIEW_WINE_LIST);
      Bind(wxEVT_MENU, &MainFrame::onQuit, this, wxID_EXIT);
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &MainFrame::onSearchCancelBtn, this);
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &MainFrame::onSearchBtn, this);
      m_search_ctrl->Bind(wxEVT_TEXT_ENTER, &MainFrame::onSearchTextEnter, this);

      if ( !wxPersistentRegisterAndRestore(this, GetName()))
      {
         // Choose some custom default size for the first run
         SetClientSize(FromDIP(wxSize(800, 600)));
         Center(wxBOTH);
      }
   }

   void MainFrame::createGridWindows()
   {
      auto box_sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);

      m_grid_options = GridOptionsPanel::create(this, m_event_source);
      box_sizer->Add(m_grid_options, wxSizerFlags(20).Expand());

      m_grid = CellarTrackerGrid::create(this, m_event_source);
      m_grid->SetMargins(0, 0);
      m_grid->SetColLabelSize(FromDIP(30));
      box_sizer->Add(m_grid, wxSizerFlags(80).Expand());

      SetSizer(box_sizer.release());
      this->SendSizeEvent();
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
         CmdId::CMD_FILE_DOWNLOAD_DATA, 
         constants::CMD_FILE_DOWNLOAD_DATA_LBL, 
         constants::CMD_FILE_DOWNLOAD_DATA_TIP, 
         wxITEM_NORMAL
      };
      menu_file->Append(menu_sync_data);
      menu_file->AppendSeparator();

      auto* menu_file_preferences = new wxMenuItem{
         menu_file, 
         CmdId::CMD_FILE_SETTINGS, 
         constants::CMD_FILE_SETTINGS_LBL,
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
         CmdId::CMD_VIEW_WINE_LIST, 
         constants::CMD_VIEWS_WINE_LIST_LBL, 
         constants::CMD_VIEWS_WINE_LIST_TIP,
         wxITEM_NORMAL
      };
      menu_views->Append(menu_views_wine_list);
      m_menu_bar->Append(menu_views, constants::MENU_VIEWS_LBL);

      SetMenuBar(m_menu_bar);

   }


   void MainFrame::createStatusBar()
   {
      constexpr int pane_count{3};

      m_status_bar = CreateStatusBar(pane_count);
      const int sb_field_widths[pane_count] = {-4, -1, -1};
      m_status_bar->SetStatusWidths(pane_count, sb_field_widths);
      const int sb_field_styles[pane_count] = {wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL};
      m_status_bar->SetStatusStyles(pane_count, sb_field_styles);
   }


   void MainFrame::createToolBar()
   {
      const auto toolbar_size = FromDIP(wxSize{24,24});

      m_tool_bar = CreateToolBar();

      auto bmp = wxBitmapBundle::FromSVGResource("TOOLBAR_DOWNLOAD", toolbar_size);
      assert(bmp.IsOk());
      m_tool_bar->AddTool(CmdId::CMD_FILE_DOWNLOAD_DATA, wxEmptyString, bmp, constants::CMD_FILE_DOWNLOAD_DATA_TIP);
      m_tool_bar->AddSeparator();

      bmp = wxBitmapBundle::FromSVGResource("TOOLBAR_SETTINGS", toolbar_size);
      assert(bmp.IsOk());
      m_tool_bar->AddTool(CmdId::CMD_FILE_SETTINGS, wxEmptyString, bmp, constants::CMD_FILE_SETTINGS_TIP);
      m_tool_bar->AddSeparator();

      m_search_ctrl = new wxSearchCtrl(m_tool_bar, wxID_ANY, wxEmptyString);
      m_search_ctrl->ShowSearchButton(true);
      m_search_ctrl->ShowCancelButton(true);
      m_tool_bar->AddControl(m_search_ctrl);

      m_tool_bar->Realize();
      m_search_ctrl->SetFocus();
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
               end_status.message = constants::ERROR_STR_DOWNLOAD_AUTH_FAILURE;
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
         if (!m_grid)
         {
            createGridWindows();
            assert(m_grid);
         }

         // load table and connect it to the event source
         GridTableLoader loader{ wxGetApp().userDataFolder() };
         auto tbl = loader.getGridTable(GridTableLoader::GridTableId::WineList);
         m_event_source->setTable(tbl);
         tbl->applySortConfig(GridTableWineList::getSortConfig(0));
         m_event_source->signal(GridTableEvent::Sort);

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
      if (!m_grid) return;
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
      if (!m_grid) return;
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
            
      int total{0};
      int filtered{0};
      
      if (m_event_source->hasTable())
      {
         auto tbl = m_event_source->getTable();
         total = tbl->totalRowCount();
         filtered = tbl->filteredRowCount();
      }

      if (total)
      {
         SetStatusText(std::format(constants::FMT_LBL_TOTAL_ROWS, total), STATUS_BAR_PANE_TOTAL_ROWS);
      }
      else{
         SetStatusText("", STATUS_BAR_PANE_TOTAL_ROWS);
      }

      if (filtered < total)
      {
         SetStatusText(std::format(constants::FMT_LBL_FILTERED_ROWS, filtered), STATUS_BAR_PANE_FILTERED_ROWS);
      }
      else{
         SetStatusText("", STATUS_BAR_PANE_FILTERED_ROWS);
      }
   }

} // namespace ctb::app
