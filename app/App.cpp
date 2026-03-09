/*********************************************************************
 * @file       App.cpp
 *
 * @brief      Implementation for the App class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "CtCredentialManager.h"
#include "HiddenWebClient.h"
#include "LabelImageCache.h"
#include "MainFrame.h"

#include <ctb/utility_http.h>
#include <ctb/tasks/tasks.h>

#include <cpr/cpr.h>
#include <wx/fileconf.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/secretstore.h>
#include <wx/xrc/xmlres.h>


#include <chrono>
#include <filesystem>
#include <thread>


namespace ctb::app
{

   App::App() 
   {
      static_cast<void>(setlocale(LC_ALL, ".UTF8")); 

      SetAppName(constants::APP_NAME_LONG);
      SetAppDisplayName(constants::APP_NAME_LONG);
      SetUseBestVisual(true);
      ::wxInitAllImageHandlers();

      auto& std_paths = wxStandardPaths::Get();
      std_paths.SetFileLayout(wxStandardPaths::FileLayout::FileLayout_XDG);

      // wxFileConfig doesn't actually create the folder for the config file on Windows, 
      // so create it first in case it doesn't exist.
      m_user_data_folder = fs::path{ std_paths.GetUserDataDir().wx_str() };
      fs::create_directories(m_user_data_folder);

      // Set up config object to use file even on windows (registry is yuck)
      auto cfg = std::make_unique<wxFileConfig>(constants::APP_NAME_LONG, 
                                                wxEmptyString, 
                                                wxEmptyString,  
                                                wxEmptyString,  
                                                wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_SUBDIR);

      using namespace log;
      auto log_folder = fs::path{ std_paths.GetUserDir(wxStandardPaths::Dir::Dir_Cache).wx_str() } / constants::APP_NAME_LONG;

#if defined(NDEBUG)
      setupDefaultLogger({{ makeFileSink(log_folder,  constants::APP_NAME_SHORT) }});
#else
      setupDefaultLogger({ { makeFileSink(log_folder,  constants::APP_NAME_SHORT) }, { makeDebuggerSink() } });
#endif

      log::info("App startup.");
      wxConfigBase::Set(cfg.release());

      // initialize label cache. needs to happen _after_ config store is set up
      m_label_cache = std::make_shared<LabelImageCache>(getLabelCacheFolder());

   } // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks) unfortunately no way around it with wxWidgets


   bool App::OnInit()
   {
      try
      {
         if (!wxApp::OnInit())
            return false;

         m_main_frame = MainFrame::create();
         m_main_frame->Show();
         m_main_frame->Bind(wxEVT_CLOSE_WINDOW, &App::onMainFrameClosed, this);
         SetTopWindow(m_main_frame);

         try
         {
            m_web_client = HiddenWebClient::create().value_or(WebClientPtr{});
            m_label_cache = std::make_shared<LabelImageCache>(getLabelCacheFolder(), m_web_client.get());
         }
         catch (...) {
            displayErrorMessage(packageError());
         }

         CallAfter([this]{wxPostEvent(m_main_frame, wxMenuEvent{ wxEVT_MENU, CmdId::CMD_COLLECTION_MY_CELLAR }); });
         return true;
      }
      catch(...){
         displayErrorMessage(packageError());
      }
      return false;
   } 

   
   int App::OnExit()
   {
      log::warn("App shutting down.");
      log::flush();
      log::shutdown();
      
#ifdef _DEBUG
      // to prevent the tzdb allocations from being reported as memory leaks
      std::chrono::get_tzdb_list().~tzdb_list();
#endif

      return wxApp::OnExit();
   }



   auto App::getLabelCacheFolder() noexcept -> fs::path
   {
      try 
      {
         auto cfg = getConfig(constants::CONFIG_PATH_PREFERENCES);
         auto val = cfg->Read(constants::CONFIG_VALUE_LABEL_CACHE_DIR, wxEmptyString).ToStdString();
         tryExpandEnvironmentVars(val);
         if (!val.empty())
         {
            return fs::path{ val };
         }
      }
      catch (...) {
         log::warn("Couldn't retrieve label cache folder from config. {}", packageError().formattedMesage());
      }
      return getDataFolder(AppFolder::Labels);
   }
   
   void App::setLabelCacheFolder(const fs::path& cache_folder)
   {
      auto new_cache = std::make_shared<LabelImageCache>(cache_folder);
      m_label_cache = new_cache;
   }


   ScopedConfigPath App::getConfig(std::string_view initial_path) noexcept(false)
   {
      auto *config = wxConfigBase::Get(false);
      if (nullptr == config)
      {
         throw Error{ constants::ERROR_STR_NO_CONFIG_STORE };
      }
      config->SetPath(wxFromSV(initial_path)); 
      return ScopedConfigPath(*config);
   }


   void App::displayErrorMessage(const Error& err, bool log_error, std::source_location source_loc)
   {
      displayErrorMessage(err.formattedMesage(), log_error, std::string{ err.categoryName() }, source_loc);
   }


   void App::displayErrorMessage(const std::string& msg, bool log_error, const std::string& title, std::source_location source_loc)
   {
      if (log_error)
      {
         log::error("Error in '{}:{}' - {}", source_loc.file_name(), source_loc.line(), msg);
      }
      wxMessageBox(msg, title, wxICON_ERROR | wxOK, m_main_frame);
   }


   void App::displayInfoMessage(const std::string& msg, const std::string& title /*= constants::APP_NAME_SHORT*/)
   {
      wxMessageBox(msg, title, wxICON_INFORMATION | wxOK, m_main_frame);
   }

   void App::onMainFrameClosed(wxCloseEvent& event)
   {
      // Need to destroy the webclient (hidden) window to prevent app from remaining in memory and prevent callback to 
      // label cache after it shuts down (in App dtor).
      m_web_client.reset();

      // Allow the default window event process to close the mainframe.
      event.Skip();
   }


}  // namespace ctb::app


// this needs to be outside the namespace for Linux but not Windows, go figure
wxIMPLEMENT_APP(ctb::app::App);
