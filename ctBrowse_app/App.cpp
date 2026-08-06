/*********************************************************************
 * @file       App.cpp
 *
 * @brief      Implementation for the App class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "MainFrame.h"

#include <wx/fileconf.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>

#include <chrono>
#include <filesystem>
#include <future>

namespace ctb::app
{

   App::App()
   {
      try
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
         setupDefaultLogger({ { makeFileSink(log_folder, constants::APP_NAME_SHORT) } });
#else
         setupDefaultLogger({ { makeFileSink(log_folder, constants::APP_NAME_SHORT) }, { makeDebuggerSink() } });
#endif

         log::info("App startup.");
         wxConfigBase::Set(cfg.release());

         configureDatasetMgr();
      }
      catch (...)
      {
         displayErrorMessage(packageError());
      }
   }   // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks) unfortunately no way around it with wxWidgets


   bool App::OnInit()
   {
      try
      {
         if (!wxApp::OnInit()) return false;

         m_main_frame = MainFrame::create();
         m_main_frame->Show();
         SetTopWindow(m_main_frame);

         CallAfter(
            [this]
            {
               wxPostEvent(m_main_frame, wxMenuEvent{ wxEVT_MENU, CmdId::CMD_COLLECTION_MY_CELLAR });
            });
         return true;
      }
      catch (...)
      {
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


   auto App::getDataFolder(AppFolder folder) const noexcept -> fs::path
   {
      if (folder == AppFolder::Root) return m_user_data_folder;

      std::string path{};
      try
      {
         // construct default path...
         auto folder_name = enum_to_string(folder);
         path             = ctb::format("{}/{}", m_user_data_folder.generic_string(), folder_name);

         // but check to make sure user hasn't overridden it.
         auto cfg = getConfig(constants::CONFIG_PATH_DATA_FOLDERS);
         path     = cfg->Read(wxFromSV(folder_name), path).ToStdString();
         tryExpandEnvironmentVars(path);

         fs::create_directories(path);
         return fs::path{ path };
      }
      catch (...)
      {
         displayFormattedMessage("Data folder '{}' does not exist and could not be created.", path);
         assert(false);
         return {};
      }
   }


   ScopedConfigPath App::getConfig(std::string_view initial_path) const noexcept(false)
   {
      auto* config = wxConfigBase::Get(false);
      if (nullptr == config)
      {
         throw Error{ constants::ERROR_STR_NO_CONFIG_STORE };
      }
      config->SetPath(wxFromSV(initial_path));
      return ScopedConfigPath(*config);
   }


   void App::displayErrorMessage(const Error& err, bool log_error, std::source_location source_loc) const
   {
      displayErrorMessage(err.formattedMessage(), log_error, std::string{ err.categoryName() }, source_loc);
   }


   void App::displayErrorMessage(const std::string& msg, bool log_error, const std::string& title, std::source_location source_loc) const
   {
      if (log_error)
      {
         log::error("Error in '{}:{}' - {}", source_loc.file_name(), source_loc.line(), msg);
      }
      wxMessageBox(msg, title, wxICON_ERROR | wxOK, m_main_frame);
   }


   void App::displayInfoMessage(const std::string& msg, const std::string& title /*= constants::APP_NAME_SHORT*/) const
   {
      wxMessageBox(msg, title, wxICON_INFORMATION | wxOK, m_main_frame);
   }

   void App::fireShutdown()
   {
      m_dataset_mgr.requestShutdown();
   }

   void App::configureDatasetMgr()
   {
      try
      {
         // currently we have a default-constructed DatasetManager without the browser running. Check if we should
         // run the browser, also set the table/image folders.
         DatasetMgrOptions opts{};

         auto config = getConfig(constants::CONFIG_PATH_PREFERENCES);
         if (!config->ReadBool(constants::CONFIG_VAL_USE_HEADLESS_BROWSER, true))
         {
            opts.browser_path.clear();   // this will keep browser from being loaded.

            // todo: in future use preferences to store custom paths for browser and its data dir, port as well.
            // for now the defaults are fine, will be easy to update later.
         }
         m_dataset_mgr.init(opts);
         m_dataset_mgr.setTableFolder(getDataFolder(AppFolder::Tables));
         m_dataset_mgr.setLabelImageFolder(getDataFolder(AppFolder::Labels));

         // initiate login verification if we have a credential.
         CtCredentialPersist cred_store{};
         if (auto cred = cred_store.loadCredential(constants::CELLARTRACKER_DOT_COM))
         {
            std::jthread{
               [this, cred = std::move(cred)] mutable -> void
               {
                  std::this_thread::sleep_for(1000ms);
                  m_dataset_mgr.checkBrowserLoginAsync(std::move(*cred),
                                                       [](LoginResult result)
                                                       {
                                                          if (result && result->first)
                                                          {
                                                             log::info("Headless Browser CT session verified for user {}", result->second);
                                                          }
                                                          else
                                                          {
                                                             std::string msg = result ? "Page loaded but couldn't log in"
                                                                                      : result.error().formattedMessage();
                                                             log::warn("Headless Browser not logged in. {}", msg);
                                                          }
                                                       });
               }
            }.detach();
         }
      }
      catch (...)   // NOLINT
      {
         log::warn("An exception was caught while initializing the dataset manager.");
         log::exception(packageError());
      }
   }


}   // namespace ctb::app


// this needs to be outside the namespace for Linux but not Windows, go figure
wxIMPLEMENT_APP(ctb::app::App);
