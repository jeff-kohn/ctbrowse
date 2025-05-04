/*********************************************************************
 * @file       MainFrame.cpp
 *
 * @brief      Implementation for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "App.h"
#include "MainFrame.h"
#include "LabelImageCache.h"
#include "CtCredentialManager.h"
#include "wx_helpers.h"
#include "dialogs/TableSyncDialog.h"
#include "model/DatasetEventSource.h"
#include "model/DatasetLoader.h"
#include "views/DatasetMultiView.h"
#include "views/DatasetOptionsPanel.h"
#include "views/DetailsPanel.h"

#include <ctb/table_download.h>
#include <ctb/utility.h>
#include <external/HttpStatusCodes.h>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/persist/toplevel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/statusbr.h>
#include <wx/stockitem.h>
#include <wx/toolbar.h>
#include <wx/wupdlock.h>
#include <wx/xrc/xmlres.h>

#include <memory>


namespace ctb::app
{

   using namespace magic_enum;

   namespace
   {
      auto eventIdToTableId(int event_id) -> TableId
      {
         switch (event_id)
         {
            case CMD_DATA_WINE_LIST:      return TableId::List;
            case CMD_DATA_PENDING_WINE:   return TableId::Pending;
            default:
               throw Error(Error::Category::ArgumentError, "Table corresponding to ID {} not found.", event_id);
         }
      }
   }

   MainFrame::MainFrame() : 
      m_event_source{ DatasetEventSource::create() },
      m_sink{ this, m_event_source },
      m_label_cache{ std::make_shared<LabelImageCache>(wxGetApp().labelCacheFolder().generic_string()) }
   {
   }


   [[nodiscard]] MainFrame* MainFrame::create()
   {
      try
      {
         // give base class a chance set up controls etc
         std::unique_ptr<MainFrame> wnd{ new MainFrame{} };

         const auto default_size = wnd->FromDIP(wxSize{800, 600});
         if (!wnd->Create(nullptr, wxID_ANY, constants::APP_NAME_LONG, wxDefaultPosition, default_size))
         {
            throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
         }
         wnd->initControls();
         return wnd.release(); // top-level window manages its own lifetime, we return non-owning pointer
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
         throw;
      }
   }


   void MainFrame::initControls()
   {
      static const auto default_window_size = wxSize{ 800, 600 };

      SetTitle(constants::APP_NAME_LONG);
      SetIcon(wxIcon{constants::RES_NAME_ICON_PRODUCT});

      // We don't actually create the child view until a dataset is opened.
      createMenuBar();
      createStatusBar();
      createToolBar();

      // Event handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuPreferences, this, CmdId::CMD_FILE_SETTINGS);
      Bind(wxEVT_MENU, &MainFrame::onMenuSyncData, this, CmdId::CMD_FILE_DOWNLOAD_DATA);
      Bind(wxEVT_MENU, &MainFrame::onMenuEditFind, this, wxID_FIND);
      Bind(wxEVT_MENU, &MainFrame::onMenuDataTable, this, CmdId::CMD_DATA_WINE_LIST);
      Bind(wxEVT_MENU, &MainFrame::onMenuDataTable, this, CmdId::CMD_DATA_PENDING_WINE);
      Bind(wxEVT_MENU, &MainFrame::onMenuViewResizeGrid, this, CmdId::CMD_VIEW_AUTOLAYOUT_COLS);
      Bind(wxEVT_MENU, &MainFrame::onQuit, this, wxID_EXIT);
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &MainFrame::onSearchCancelBtn, this);
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &MainFrame::onSearchBtn, this);
      m_search_ctrl->Bind(wxEVT_TEXT_ENTER, &MainFrame::onSearchTextEnter, this);
      m_search_ctrl->Bind(wxEVT_KEY_DOWN, &MainFrame::onSearchKeyDown, this);

      if ( !wxPersistentRegisterAndRestore(this, constants::RES_NAME_MAINFRAME) )
      {
         Center(wxBOTH);
      }
   }


   void MainFrame::createMenuBar()
   {
      using namespace constants;

      m_menu_bar = new wxMenuBar();

      // File menu
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
      auto* menu_edit = new wxMenu();
      auto* menu_edit_find = new wxMenuItem(menu_edit, wxID_FIND);
      menu_edit_find->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FIND, wxART_MENU));
      menu_edit->Append(menu_edit_find);
      m_menu_bar->Append(menu_edit, wxGetStockLabel(wxID_EDIT));


      // Data Menu
      auto* menu_data = new wxMenu();
      menu_data->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_DATA_WINE_LIST, 
         constants::CMD_DATA_WINE_LIST_LBL, 
         constants::CMD_DATA_WINE_LIST_TIP,
         wxITEM_NORMAL
      });
      menu_data->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_DATA_PENDING_WINE, 
         constants::CMD_DATA_PENDING_WINE_LBL, 
         constants::CMD_DATA_PENDING_WINE_TIP,
         wxITEM_NORMAL
         });
      m_menu_bar->Append(menu_data, constants::LBL_MENU_DATA);


      // Wine Menu
      auto* menu_wine = new wxMenu();
      menu_wine->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_WINE_ONLINE_DETAILS, 
         constants::CMD_WINE_ONLINE_DETAILS_LBL, 
         constants::CMD_WINE_ONLINE_DETAILS_TIP,
         wxITEM_NORMAL
      });
      menu_wine->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_WINE_ONLINE_VINTAGES, 
         constants::CMD_WINE_ONLINE_VINTAGES_LBL, 
         constants::CMD_WINE_ONLINE_VINTAGES_TIP,
         wxITEM_NORMAL
         });
      menu_wine->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_WINE_ONLINE_PRODUCER, 
         constants::CMD_WINE_ONLINE_PRODUCER_LBL, 
         constants::CMD_WINE_ONLINE_PRODUCER_TIP,
         wxITEM_NORMAL
      });
      menu_wine->AppendSeparator();
      menu_wine->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_WINE_ONLINE_ACCEPT_WINE, 
         constants::CMD_WINE_ONLINE_ACCEPT_WINE_LBL, 
         constants::CMD_WINE_ONLINE_ACCEPT_WINE_TIP,
         wxITEM_NORMAL
      });
      m_menu_bar->Append(menu_wine, constants::LBL_MENU_WINE);

      // View Menu
      auto* menu_view = new wxMenu();
      menu_view->Append(new wxMenuItem{
         menu_view, 
         CmdId::CMD_VIEW_AUTOLAYOUT_COLS, 
         constants::CMD_VIEW_AUTOLAYOUT_COLS_LBL, 
         constants::CMD_VIEW_AUTOLAYOUT_COLS_TIP,
         wxITEM_NORMAL
         });
      m_menu_bar->Append(menu_view, constants::LBL_MENU_VIEW);


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
      const auto toolbar_size = wxSize{24,24};

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
      try
      {
         m_search_ctrl->SetFocus();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuPreferences([[maybe_unused]] wxCommandEvent& event)
   {

   }


   void MainFrame::onMenuSyncData([[maybe_unused]] wxCommandEvent& event) 
   {
      // TODO: refactor this
      try
      {

         TableSyncDialog dlg(this);
         if (dlg.ShowModal() != wxID_OK)
            return;

         wxBusyCursor busy{};
         ScopedStatusText end_status{ constants::STATUS_DOWNLOAD_COMPLETE, this };

         CtCredentialManager cred_mgr{};
         auto cred_name = constants::CELLARTRACKER_DOT_COM;
         auto prompt_msg = ctb::format(constants::FMT_CREDENTIALDLG_PROMPT_MSG, cred_name);

         auto cred_result = cred_mgr.loadCredential(cred_name, prompt_msg, true);

         if (!cred_result)
         {
            auto error = cred_result.error();
            if (error.category == Error::Category::OperationCanceled)
               return;

            throw error;
         }

         // If cred doesn't work we need to reprompt so udpate prompt message.
         prompt_msg = ctb::format(constants::FMT_CREDENTIALDLG_REPROMPT_MSG, cred_name);

         wxProgressDialog progress_dlg{"Download Progress", "Downloading Data Files", 100, this, wxPD_CAN_ABORT | wxPD_AUTO_HIDE | wxPD_APP_MODAL };

         ProgressCallback progress_callback = [&progress_dlg] ([[maybe_unused]] int64_t downloadTotal, [[maybe_unused]] int64_t downloadNow,
                                                                     [[maybe_unused]] int64_t uploadTotal, [[maybe_unused]] int64_t uploadNow,
                                                                     [[maybe_unused]] intptr_t userdata)
                                                                     {
                                                                        return progress_dlg.Pulse();
                                                                     };

         // For each selected table, download it.
         for (auto tbl : dlg.selectedTables())
         {
            setStatusText(constants::FMT_STATUS_FILE_DOWNLOADING, getTableDescription(tbl));

            DownloadResult result{};
            result = downloadRawTableData(*cred_result, tbl, DataFormatId::csv, &progress_callback);

            while (!result)
            {
               auto error = result.error();
               if (error.error_code == std::to_underlying(HttpStatus::Code::Unauthorized))
               {
                  // login failure, need to re-prompt for credentials.
                  cred_result = cred_mgr.promptCredential(cred_name, prompt_msg, true);
                  if (cred_result)
                  {
                     // got a new cred, try again
                     result = downloadRawTableData(*cred_result, tbl, DataFormatId::csv, &progress_callback);
                  }
                  else{
                     // user canceled login dialog so just exit out
                     end_status.message = constants::ERROR_STR_DOWNLOAD_AUTH_FAILURE;
                     return;
                  }
               }
               else if (error.category == Error::Category::OperationCanceled)
               {
                  // user hit cancel in progress dilaog, so just exit out.
                  end_status.message = constants::STATUS_DOWNLOAD_CANCELED;
                  return;
               }               
               else {
                  // some unknown error happened, let user know before bailing.
                  wxGetApp().displayErrorMessage(result.error());
                  end_status.message = constants::STATUS_DOWNLOAD_FAILED;
                  return;
               }
            }
            // did user ask to save cred?
            if (cred_result->saveRequested())
            {
               cred_mgr.saveCredential(*cred_result);
            }

            // if we get here we have the data, so save it to file.
            auto folder = wxGetApp().userDataFolder();
            auto file_path{ folder / result->tableName() };
            file_path.replace_extension(constants::DATA_FILE_EXTENSION);
            saveTextToFile(file_path, result->data, true);

            setStatusText(constants::FMT_STATUS_FILE_DOWNLOADED, getTableDescription(tbl));
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }

   }


   void MainFrame::onMenuDataTable([[maybe_unused]] wxCommandEvent& event)
   {
      wxBusyCursor busy{};
      wxWindowUpdateLocker lock{ this };
      try
      {
         if (!m_view)
         {
            m_view = DatasetMultiView::create(this, m_event_source, m_label_cache);
         }


         // load table and connect it to the event source
         DatasetLoader loader{ wxGetApp().userDataFolder() };
         auto table_id = eventIdToTableId(event.GetId());;
         auto tbl = loader.getDataset(table_id);

         // Apply in-stock filter by default?
         if (wxGetApp().getConfig(constants::CONFIG_PATH_PREFERENCES)->ReadBool(constants::CONFIG_VALUE_DEFAULT_IN_STOCK_ONLY, constants::CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT))
         {
            tbl->setInStockFilter(true);
         }
         m_event_source->setTable(tbl, true);

         // Force a complete redraw of everything
         Layout();
         SendSizeEvent();
         Update();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onSearchBtn([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         doSearchFilter();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuViewResizeGrid(wxCommandEvent&)
   {
   }


   void MainFrame::onSearchCancelBtn([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         clearSearchFilter();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onSearchTextEnter([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         doSearchFilter();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onSearchKeyDown(wxKeyEvent& event)
   {
      try
      {
         switch (event.GetKeyCode())
         {
            case WXK_TAB:
               m_view->SetFocus();
               break;

            case WXK_ESCAPE:
               clearSearchFilter();
               break;

            default:
               event.Skip();
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onQuit([[maybe_unused]] wxCommandEvent& event)
   {
      Close(true);
   }


   void MainFrame::doSearchFilter()
   {
      if (!m_event_source->hasTable()) return;
      try
      {
         auto dataset = m_event_source->getTable();
         if (dataset->filterBySubstring(m_search_ctrl->GetValue().wx_str()))
         {
            m_event_source->signal(DatasetEvent::Id::SubStringFilter);
         }
         else {
            wxGetApp().displayInfoMessage(constants::INFO_MSG_NO_MATCHING_ROWS);
            m_search_ctrl->SetFocus();
            m_search_ctrl->SelectAll();
         }
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
      updateStatusBarCounts();
   }


   void MainFrame::clearSearchFilter()
   {
      if (!m_event_source->hasTable()) return;
      try
      {
         m_search_ctrl->ChangeValue("");
         m_event_source->getTable()->clearSubStringFilter();
         m_event_source->signal(DatasetEvent::Id::Filter);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError());
      }
      updateStatusBarCounts();
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
         SetStatusText(ctb::format(constants::FMT_LBL_TOTAL_ROWS, total), STATUS_BAR_PANE_TOTAL_ROWS);
      }
      else{
         SetStatusText("", STATUS_BAR_PANE_TOTAL_ROWS);
      }

      if (filtered < total)
      {
         SetStatusText(ctb::format(constants::FMT_LBL_FILTERED_ROWS, filtered), STATUS_BAR_PANE_FILTERED_ROWS);
      }
      else{
         SetStatusText("", STATUS_BAR_PANE_FILTERED_ROWS);
      }
   }


   void MainFrame::notify([[maybe_unused]] DatasetEvent event)
   {
      updateStatusBarCounts();
   }

} // namespace ctb::app
