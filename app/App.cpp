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
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>

#include <chrono>
#include <filesystem>



namespace ctb::app
{
   namespace fs = std::filesystem;

   namespace
   {
      const wxSize g_default_frame_size{ 400, 600 }; // NOLINT(cert-err58-cpp) this ctor doesn't throw it's just not marked noexcept

   } //namespace


   App::App()
   {
      log::setupDefaultLogger();
      log::info("App startup.");

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

      wxConfigBase::Set(cfg.release());

   } // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks) unfortunately no way around it with wxWidgets


   bool App::OnInit()
   {
      try
      {
         if (!wxApp::OnInit())
            return false;

         m_main_frame = MainFrame::create();
         m_main_frame->Show();
         SetTopWindow(m_main_frame);

         return true;
      }
      catch(Error& err)
      {
         displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         displayErrorMessage(e.what());
      }
      return false;
   } 


   int App::OnExit()
   {
#ifdef _DEBUG
      // to prevent the tzdb allocations from being reported as memory leaks
      std::chrono::get_tzdb_list().~tzdb_list();
#endif
      log::warn("App shutting down.");
      log::flush();
      log::shutdown();
      return wxApp::OnExit();
   }


   ScopedConfigPath App::getConfig() noexcept(false)
   {
      auto *config = wxConfigBase::Get(false);
      if (nullptr == config)
      {
         throw Error{ "No configuration object available" };
      }
      config->SetPath("/"); // the current path is persistent/global, which is fucking stupid.
      return ScopedConfigPath(*config);
   }


   void App::displayErrorMessage(const Error& err)
   {
      auto title = ctb::format(constants::FMT_TITLE_TYPED_ERROR, err.categoryName());
      displayErrorMessage(err.error_message, title);
   }


   void App::displayErrorMessage(const std::string& msg, const std::string& title)
   {
      wxMessageBox(msg, title, wxICON_ERROR | wxOK, m_main_frame);
   }


   void App::displayInfoMessage(const std::string& msg, const std::string& title /*= constants::APP_NAME_SHORT*/)
   {
      wxMessageBox(msg, title, wxICON_INFORMATION | wxOK, m_main_frame);
   }


}  // namespace ctb::app


// this needs to be outside the namespace for Linux but not Windows, go figure
wxIMPLEMENT_APP(ctb::app::App);
