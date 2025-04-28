/*********************************************************************
 * @file       App.cpp
 *
 * @brief      Implementation for the App class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "MainFrame.h"

#include <ctb/utility_http.h>

#include <cpr/cpr.h>
#include <wx/fileconf.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/secretstore.h>
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
      setupDefaultLogger({{ makeFileSink(log_folder) }});
#else
      setupDefaultLogger({ {makeFileSink(log_folder)}, {makeDebuggerSink()} });
#endif

      log::info("App startup.");
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

         CallAfter([this]{wxPostEvent(m_main_frame, wxMenuEvent{ wxEVT_MENU, CmdId::CMD_VIEW_WINE_LIST }); });

         return true;
      }
      catch(...){
         displayErrorMessage(packageError());
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


   //auto App::getCellarTrackerCookies(bool prompt_for_cred) -> CookieResult
   //{
   //   if (m_ct_cookies) return *m_ct_cookies;

   //   try
   //   {
   //      cpr::Url url{ "https://www.cellartracker.com/password.asp" };
   //      cpr::Payload form_data{
   //         { "Referrer", "/default.asp" },
   //         { "szUser", "Jeff Kohn" },
   //         { "szPassword", "lkj243df" },
   //         { "UseCookie", "true" } };

   //      auto response = cpr::Post(url, form_data, getDefaultHeaders());

   //      // check the response for success, bail out if we got an error 
   //      auto request_result = validateResponse(response);
   //      if (!request_result.has_value())
   //      {
   //         throw request_result.error();
   //      }
   //   }
   //   catch(...){
   //      wxGetApp().displayErrorMessage(packageError(), true);
   //   }
   //}

   auto App::labelCacheFolder() noexcept -> fs::path
   {
      try 
      {
         auto cfg = getConfig(constants::CONFIG_PATH_PREFERENCES);
         auto val = cfg->Read(wxString{ constants::CONFIG_VALUE_LABEL_CACHE_DIR }, wxEmptyString);
         if (!val.empty())
         {
            return fs::path{ val.wx_str() };
         }
      }
      catch (...) {
         log::warn("Couldn't retrieve label cache folder from config. {}", packageError().formattedMesage());
      }
      return userDataFolder() / constants::APP_LABELS_SUBFOLDER;
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
      auto title = ctb::format(constants::FMT_TITLE_TYPED_ERROR, err.categoryName());
      displayErrorMessage(err.formattedMesage(), log_error, title, source_loc);
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


}  // namespace ctb::app


// this needs to be outside the namespace for Linux but not Windows, go figure
wxIMPLEMENT_APP(ctb::app::App);
