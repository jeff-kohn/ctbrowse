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
#include "views/DatasetMultiView.h"
#include "views/DatasetOptionsPanel.h"
#include "views/DetailsPanel.h"

#include <ctb/utility.h>
#include <ctb/utility_chrono.h>
#include <ctb/utility_http.h>
#include <ctb/table_download.h>
#include <ctb/model/DatasetEventSource.h>
#include <ctb/model/CtDatasetLoader.h>

#include <external/HttpStatusCodes.h>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/filedlg.h>
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
            case CMD_COLLECTION_MY_CELLAR:      return TableId::List;
            case CMD_COLLECTION_PENDING_WINE:   return TableId::Pending;
            case CMD_COLLECTION_READY_TO_DRINK: return TableId::Availability;
            case CMD_COLLECTION_CONSUMED:       return TableId::Consumed;
            default:
               throw Error(Error::Category::ArgumentError, "Table corresponding to ID {} not found.", event_id);
         }
      }


      /// @brief Load a dataset from disk, and optionally apply saved options
      auto loadDataset(TableId table_id) -> DatasetPtr
      {
         // load dataset and then apply options.
         CtDatasetLoader loader{ wxGetApp().getDataFolder() };
         return loader.getDataset(table_id);
      }

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


   auto MainFrame::getWinePopupMenu() const -> wxMenuPtr
   {
      if (!m_event_source->hasDataset())
         return {};


      // build our popup menu based first on the universal commands then the property/dataset-specific ones.
      auto popup_menu = std::make_unique<wxMenu>();
      popup_menu->Append(new wxMenuItem{ popup_menu.get(),
         CmdId::CMD_ONLINE_WINE_DETAILS,
         constants::CMD_ONLINE_VIEW_ON_CT_LBL,
         constants::CMD_ONLINE_VIEW_ON_CT_TIP,
         wxITEM_NORMAL
         });

      popup_menu->Append(new wxMenuItem{ popup_menu.get(),
         CmdId::CMD_ONLINE_SEARCH_VINTAGES,
         constants::CMD_ONLINE_SEARCH_VINTAGES_LBL,
         constants::CMD_ONLINE_SEARCH_VINTAGES_TIP,
         wxITEM_NORMAL
         });

      popup_menu->AppendSeparator();
      popup_menu->Append(new wxMenuItem{ popup_menu.get(), 
         CmdId::CMD_ONLINE_DRINK_WINDOW, 
         constants::CMD_ONLINE_DRINK_WINDOW_LBL, 
         constants::CMD_ONLINE_DRINK_WINDOW_TIP,
         wxITEM_NORMAL
         });

      popup_menu->AppendSeparator();
      popup_menu->Append(new wxMenuItem{ popup_menu.get(),
         CmdId::CMD_ONLINE_ADD_TASTING_NOTE,
         constants::CMD_ONLINE_ADD_TASTING_NOTE_LBL,
         constants::CMD_ONLINE_ADD_TASTING_NOTE_LBL,
         wxITEM_NORMAL
         });

      auto dataset = m_event_source->getDataset();
      if (dataset->getTableId() != TableId::Pending) // could be confusing whether accepting pending or adding new order
      {
         popup_menu->Append(new wxMenuItem{ popup_menu.get(),
            CmdId::CMD_ONLINE_ADD_TO_CELLAR,
            constants::CMD_ONLINE_ADD_TO_CELLAR_LBL,
            constants::CMD_ONLINE_ADD_TO_CELLAR_TIP,
            wxITEM_NORMAL
            });
      }

      if (dataset->hasProperty(CtProp::QtyOnHand))  // can only drink if have can check available inventory
      {
         popup_menu->AppendSeparator();
         popup_menu->Append(new wxMenuItem{
            popup_menu.get(),
            CmdId::CMD_ONLINE_DRINK_REMOVE, 
            constants::CMD_ONLINE_DRINK_REMOVE_LBL, 
            constants::CMD_ONLINE_DRINK_REMOVE_LBL,
            wxITEM_NORMAL
            });
      }
      
      // only show these in Pending view for now, might want to reconsider for other views that have pending qtry
      if (dataset->getTableId() == TableId::Pending)  
      {
         popup_menu->AppendSeparator();
         popup_menu->Append(new wxMenuItem{
            popup_menu.get(),
            CmdId::CMD_ONLINE_ACCEPT_PENDING,
            constants::CMD_ONLINE_ACCEPT_PENDING_LBL,
            constants::CMD_ONLINE_ACCEPT_PENDING_TIP,
            wxITEM_NORMAL
            });
         popup_menu->Append(new wxMenuItem{
            popup_menu.get(),
            CmdId::CMD_ONLINE_EDIT_ORDER,
            constants::CMD_ONLINE_EDIT_ORDER_LBL,
            constants::CMD_ONLINE_EDIT_ORDER_LBL,
            wxITEM_NORMAL
            });
      }

      return popup_menu;
   }

   
   MainFrame::MainFrame() :
      m_event_source{ DatasetEventSource::create() },
      m_sink{ this, m_event_source },
      m_label_cache{ std::make_shared<LabelImageCache>(wxGetApp().labelCacheFolder().generic_string()) }
   {
   }


   void MainFrame::initControls()
   {
      static const auto default_window_size = wxSize{ 800, 600 };

      SetTitle(constants::APP_NAME_LONG);
      SetIcon(wxIcon{constants::RES_NAME_ICON_PRODUCT});

      // We don't actually create the child view until a dataset is opened.
      createMenuBar();
      createToolBar();
      m_status_bar = CreateStatusBar();

      // File menu handlers
      //Bind(wxEVT_MENU, &MainFrame::onMenuFilePreferences, this, CmdId::CMD_FILE_SETTINGS);
      Bind(wxEVT_MENU, &MainFrame::onMenuFileOpen,        this, CmdId::CMD_FILE_OPEN);
      Bind(wxEVT_MENU, &MainFrame::onMenuFileSave,        this, CmdId::CMD_FILE_SAVE);
      Bind(wxEVT_MENU, &MainFrame::onMenuFileSyncData,    this, CmdId::CMD_FILE_DOWNLOAD_DATA);
      Bind(wxEVT_MENU, &MainFrame::onMenuFileSyncData,    this, CmdId::CMD_FILE_DOWNLOAD_DATA);
      Bind(wxEVT_MENU, &MainFrame::onMenuFileQuit,        this, wxID_EXIT);

      // Edit menu handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuEditFind, this, wxID_FIND);

      // Collection menu handlers
      Bind(wxEVT_MENU, &MainFrame::onMenuCollection, this, CmdId::CMD_COLLECTION_MY_CELLAR);
      Bind(wxEVT_MENU, &MainFrame::onMenuCollection, this, CmdId::CMD_COLLECTION_PENDING_WINE);
      Bind(wxEVT_MENU, &MainFrame::onMenuCollection, this, CmdId::CMD_COLLECTION_READY_TO_DRINK);
      Bind(wxEVT_MENU, &MainFrame::onMenuCollection, this, CmdId::CMD_COLLECTION_CONSUMED);

      // Online menu events
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineWineDetails,    this, CMD_ONLINE_WINE_DETAILS);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineSearchVintages, this, CMD_ONLINE_SEARCH_VINTAGES);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineDrinkWindow,    this, CMD_ONLINE_DRINK_WINDOW);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineAddToCellar,    this, CMD_ONLINE_ADD_TO_CELLAR);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineAddTastingNote, this, CMD_ONLINE_ADD_TASTING_NOTE);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineAcceptDelivery, this, CMD_ONLINE_ACCEPT_PENDING);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineEditOrder,      this, CMD_ONLINE_EDIT_ORDER);
      Bind(wxEVT_MENU, &MainFrame::onMenuOnlineDrinkRemove,    this, CMD_ONLINE_DRINK_REMOVE);

      // UI update handlers for online menu commands
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineWineSelectionUI,  this, CMD_ONLINE_WINE_DETAILS);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineWineSelectionUI,  this, CMD_ONLINE_SEARCH_VINTAGES);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineWineSelectionUI,  this, CMD_ONLINE_DRINK_WINDOW);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineWineSelectionUI,  this, CMD_ONLINE_ADD_TASTING_NOTE);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineAddToCellarUI,    this, CMD_ONLINE_ADD_TO_CELLAR);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineAcceptDeliveryUI, this, CMD_ONLINE_EDIT_ORDER);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineAcceptDeliveryUI, this, CMD_ONLINE_ACCEPT_PENDING);
      Bind(wxEVT_UPDATE_UI, &MainFrame::onMenuOnlineDrinkRemoveUI,    this, CMD_ONLINE_DRINK_REMOVE);

      // Toolbar event handlers
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &MainFrame::onToolbarSearchCancelBtn, this);
      m_search_ctrl->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &MainFrame::onToolbarSearchBtn,       this);
      m_search_ctrl->Bind(wxEVT_TEXT_ENTER,            &MainFrame::onToolbarSearchTextEnter, this);
      m_search_ctrl->Bind(wxEVT_KEY_DOWN,              &MainFrame::onToolbarSearchKeyDown,   this);

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

      auto* menu_load_options = new wxMenuItem{
         menu_file, 
         CmdId::CMD_FILE_OPEN, 
         constants::CMD_FILE_OPEN_LBL, 
         constants::CMD_FILE_OPEN_TIP, 
         wxITEM_NORMAL
      };
      menu_file->Append(menu_load_options);

      auto* menu_save_options = new wxMenuItem{
         menu_file, 
         CmdId::CMD_FILE_SAVE, 
         constants::CMD_FILE_SAVE_LBL, 
         constants::CMD_FILE_SAVE_TIP, 
         wxITEM_NORMAL
      };
      menu_file->Append(menu_save_options);
      menu_file->AppendSeparator();

      auto* menu_sync_data = new wxMenuItem{
         menu_file, 
         CmdId::CMD_FILE_DOWNLOAD_DATA, 
         constants::CMD_FILE_DOWNLOAD_DATA_LBL, 
         constants::CMD_FILE_DOWNLOAD_DATA_TIP, 
         wxITEM_NORMAL
      };
      menu_file->Append(menu_sync_data);
      menu_file->AppendSeparator();

      //auto* menu_file_preferences = new wxMenuItem{
      //   menu_file, 
      //   CmdId::CMD_FILE_SETTINGS, 
      //   constants::CMD_FILE_SETTINGS_LBL,
      //   constants::CMD_FILE_SETTINGS_TIP,
      //   wxITEM_NORMAL
      //};
      //menu_file->Append(menu_file_preferences);
      //menu_file->AppendSeparator();

      auto* menu_file_quit = new wxMenuItem(menu_file, wxID_EXIT);
      menu_file->Append(menu_file_quit);
      m_menu_bar->Append(menu_file, wxGetStockLabel(wxID_FILE));
   

      // Edit Menu
      auto* menu_edit = new wxMenu();
      auto* menu_edit_find = new wxMenuItem(menu_edit, wxID_FIND);
      menu_edit_find->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FIND, wxART_MENU));
      menu_edit->Append(menu_edit_find);
      m_menu_bar->Append(menu_edit, wxGetStockLabel(wxID_EDIT));


      // Collection Menu
      auto* menu_data = new wxMenu();
      menu_data->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_COLLECTION_MY_CELLAR, 
         constants::CMD_COLLECTION_MY_CELLAR_LBL, 
         constants::CMD_COLLECTION_MY_CELLAR_TIP,
         wxITEM_NORMAL
      });
      menu_data->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_COLLECTION_PENDING_WINE, 
         constants::CMD_COLLECTION_PENDING_WINE_LBL, 
         constants::CMD_COLLECTION_PENDING_WINE_TIP,
         wxITEM_NORMAL
         });
      menu_data->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_COLLECTION_CONSUMED, 
         constants::CMD_COLLECTION_CONSUMED_LBL, 
         constants::CMD_COLLECTION_CONSUMED_TIP,
         wxITEM_NORMAL
         });
      menu_data->AppendSeparator();
      menu_data->Append(new wxMenuItem{
         menu_data, 
         CmdId::CMD_COLLECTION_READY_TO_DRINK, 
         constants::CMD_COLLECTION_READY_TO_DRINK_LBL, 
         constants::CMD_COLLECTION_READY_TO_DRINK_TIP,
         wxITEM_NORMAL
         });
      m_menu_bar->Append(menu_data, constants::LBL_MENU_COLLECTION);


      // Wine Menu
      auto* menu_wine = new wxMenu();
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_WINE_DETAILS, 
         constants::CMD_ONLINE_WINE_DETAILS_LBL, 
         constants::CMD_ONLINE_WINE_DETAILS_TIP,
         wxITEM_NORMAL
      });
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_SEARCH_VINTAGES, 
         constants::CMD_ONLINE_SEARCH_VINTAGES_LBL, 
         constants::CMD_ONLINE_SEARCH_VINTAGES_TIP,
         wxITEM_NORMAL
      });
      menu_wine->AppendSeparator();
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_DRINK_WINDOW, 
         constants::CMD_ONLINE_DRINK_WINDOW_LBL, 
         constants::CMD_ONLINE_DRINK_WINDOW_TIP,
         wxITEM_NORMAL
      });
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_ADD_TO_CELLAR, 
         constants::CMD_ONLINE_ADD_TO_CELLAR_LBL, 
         constants::CMD_ONLINE_ADD_TO_CELLAR_TIP,
         wxITEM_NORMAL
         });
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_ADD_TASTING_NOTE, 
         constants::CMD_ONLINE_ADD_TASTING_NOTE_LBL, 
         constants::CMD_ONLINE_ADD_TASTING_NOTE_LBL,
         wxITEM_NORMAL
      });

      menu_wine->AppendSeparator();
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_ACCEPT_PENDING, 
         constants::CMD_ONLINE_ACCEPT_PENDING_LBL, 
         constants::CMD_ONLINE_ACCEPT_PENDING_TIP,
         wxITEM_NORMAL
      });
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_EDIT_ORDER, 
         constants::CMD_ONLINE_EDIT_ORDER_LBL, 
         constants::CMD_ONLINE_EDIT_ORDER_LBL,
         wxITEM_NORMAL
      });
      menu_wine->AppendSeparator();
      menu_wine->Append(new wxMenuItem{
         menu_wine, 
         CmdId::CMD_ONLINE_DRINK_REMOVE, 
         constants::CMD_ONLINE_DRINK_REMOVE_LBL, 
         constants::CMD_ONLINE_DRINK_REMOVE_LBL,
         wxITEM_NORMAL
      });

      m_menu_bar->Append(menu_wine, constants::LBL_MENU_WINE);
      SetMenuBar(m_menu_bar);
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


   void MainFrame::onMenuFilePreferences([[maybe_unused]] wxCommandEvent& event)
   {

   }


   void MainFrame::onMenuFileSave(wxCommandEvent&)
   {
      try 
      {
         auto dataset = getDataset();

         wxFileDialog save_dialog{
            this,
            constants::FILE_OPEN_COLLECTION_FILTER,
            wxGetApp().getDataFolder(AppFolder::Favorites).generic_string(),
            dataset->getCollectionName(),
            constants::FILE_COLLECTION_CTBC_FILTER,
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT
         };

         if (save_dialog.ShowModal() == wxID_OK)
         {
            fs::path file_path = save_dialog.GetPath().utf8_string();
            dataset->setCollectionName(file_path.stem().generic_string());
            auto options = CtDatasetOptions::retrieveOptions(dataset);
            CtDatasetOptions::saveOptions(options, file_path, true);
            SetTitle(ctb::format("{} - {}", dataset->getCollectionName(), constants::APP_NAME_LONG));
         }
      }
      catch (...) 
      {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuFileOpen(wxCommandEvent&)
   {
      try 
      {
         wxFileDialog open_dialog{
            this,
            constants::FILE_OPEN_COLLECTION_FILTER,
            wxGetApp().getDataFolder(AppFolder::Favorites).generic_string(),
            wxEmptyString,
            constants::FILE_COLLECTION_CTBC_FILTER,
            wxFD_OPEN | wxFD_FILE_MUST_EXIST
         };

         if (open_dialog.ShowModal() == wxID_CANCEL)
            return;

         auto path = open_dialog.GetPath().utf8_string();
         auto options = CtDatasetOptions::retrieveOptions(path);
         auto dataset = loadDataset(options.table_id);
         options.applyToDataset(dataset);
         setDataset(dataset);
      }
      catch (...) 
      {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuFileSyncData([[maybe_unused]] wxCommandEvent& event) 
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
            auto folder = wxGetApp().getDataFolder();
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


   void MainFrame::onMenuFileQuit([[maybe_unused]] wxCommandEvent& event)
   {
      Close(true);
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


   void MainFrame::onMenuCollection([[maybe_unused]] wxCommandEvent& event)
   {
      wxBusyCursor busy{};
      wxWindowUpdateLocker lock{ this };
      try
      {
         if (!m_view)
         {
            m_view = DatasetMultiView::create(*this, m_event_source, m_label_cache);
         }

         // apply any previously-saved default settings before attaching to source
         auto dataset = loadDataset(eventIdToTableId(event.GetId()));
         CtDatasetOptions::applyDefaultOptions(dataset);
         setDataset(dataset);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineWineDetails(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         wxLaunchDefaultBrowser(getWineDetailsUrl(wine_id));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineSearchVintages(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine = dataset->getProperty(m_selected_row, CtProp::WineName).asString();
         wxLaunchDefaultBrowser(getWineVintagesUrl(wine));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineDrinkWindow(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         wxLaunchDefaultBrowser(getDrinkWindowUrl(wine_id));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineAddToCellar(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         wxLaunchDefaultBrowser(getAddToCellarUrl(wine_id));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineAddTastingNote(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         wxLaunchDefaultBrowser(getAddTastingNoteUrl(wine_id));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineAcceptDelivery(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         auto purchase_id = dataset->getProperty(m_selected_row, CtProp::PendingPurchaseId).asStringView();
         wxLaunchDefaultBrowser(getAcceptPendingUrl(wine_id, purchase_id, getCalendarDate()));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineEditOrder(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         auto purchase_id = dataset->getProperty(m_selected_row, CtProp::PendingPurchaseId).asStringView();
         wxLaunchDefaultBrowser(getEditPendingUrl(wine_id, purchase_id));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineDrinkRemove(wxCommandEvent&)
   {
      try
      {
         auto dataset = getDataset();
         auto wine_id = dataset->getProperty(m_selected_row, CtProp::iWineId).asStringView();
         wxLaunchDefaultBrowser(getDrinkRemoveUrl(wine_id));
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onMenuOnlineWineSelectionUI(wxUpdateUIEvent& event)
   {
      // Enable only when a wine row is selected
      event.Enable(m_selected_row >= 0);
   }


   void MainFrame::onMenuOnlineAddToCellarUI(wxUpdateUIEvent& event)
   {
      auto dataset = getDataset(false);
      bool enable = (m_selected_row >= 0) and dataset.get() and (dataset->getTableId() != TableId::Pending);
      event.Enable(enable);
   }


   void MainFrame::onMenuOnlineAcceptDeliveryUI(wxUpdateUIEvent& event)
   {
      auto dataset = getDataset(false);
      bool enable = (m_selected_row >= 0) and dataset.get() and (dataset->getTableId() == TableId::Pending);
      event.Enable(enable);
   }


   void MainFrame::onMenuOnlineDrinkRemoveUI(wxUpdateUIEvent& event)
   {
      auto dataset = getDataset(false);
      bool enable = (m_selected_row >= 0) and dataset.get() and (dataset->getProperty(m_selected_row, CtProp::QtyOnHand).asInt32().value_or(0) > 0);
      event.Enable(enable);
   }


   void MainFrame::onToolbarSearchBtn([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         doSearchFilter();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onToolbarSearchCancelBtn([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         clearSearchFilter();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::onToolbarSearchKeyDown(wxKeyEvent& event)
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


   void MainFrame::onToolbarSearchTextEnter([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         doSearchFilter();
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void MainFrame::clearSearchFilter()
   {
      if (!m_event_source->hasDataset()) return;
      try
      {
         m_search_ctrl->ChangeValue("");
         m_event_source->getDataset()->clearSubStringFilter();
         m_event_source->signal(DatasetEvent::Id::Filter);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
      updateStatusBarCounts();
   }


   void MainFrame::doSearchFilter()
   {
      if (!m_event_source->hasDataset()) return;
      try
      {
         auto dataset = m_event_source->getDataset();
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


   auto MainFrame::getDataset(bool throw_on_null) -> DatasetPtr
   {
      auto dataset = m_event_source->getDataset();
      if (nullptr == dataset.get() and throw_on_null)
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      return dataset;
   }


   void MainFrame::setDataset(DatasetPtr dataset)
   {
      m_event_source->setDataset(dataset, true);

      // Update title bar
      SetTitle(ctb::format("{} - {}", dataset->getCollectionName(), constants::APP_NAME_LONG));

      // Force a complete redraw of everything
      Layout();
      SendSizeEvent();
      Update();
   }


   void MainFrame::updateStatusBarCounts()
   {     
      std::string summary{};
      if (m_event_source->hasDataset())
      {
         summary = m_event_source->getDataset()->getDataSummary();
      }
      SetStatusText(summary);
   }


   void MainFrame::notify([[maybe_unused]] DatasetEvent event)
   {
      constexpr int none = -1;

      switch (event.event_id)
      {
         case DatasetEvent::Id::RowSelected:
            m_selected_row = event.affected_row.value_or(none);
            break;

         case DatasetEvent::Id::DatasetRemove:
            break;

         default:
            m_selected_row = none;
      }
      updateStatusBarCounts();
   }

} // namespace ctb::app

