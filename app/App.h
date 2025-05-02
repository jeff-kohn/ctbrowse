/*********************************************************************
 * @file       App.h
 *
 * @brief      Declaration for the App class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "app_constants.h"
#include "LoginEvent.h"
#include "wx_helpers.h"

#include <ctb/log.h>
#include <wx/app.h>

#include <expected>


namespace ctb::app
{
   namespace fs = std::filesystem;


   /// @brief forward declare top-level window class so we don't have to add header dependency
   ///
   class MainFrame;


   /// @brief app object for the application.
   ///
   class App : public wxApp
   {
   public:
      App();

      /// @brief called by the framework on app startup, this is the place for program initialization 
      ///
      auto OnInit() -> bool override;

      /// @brief called by the framework on app shutdown, this is the place for resource cleanup and other shutdown tasks
      ///
      auto OnExit() -> int override;

      /// @brief  returns the path where the application stores data files.
      ///
      auto userDataFolder() const noexcept  -> const fs::path& 
      {
         return m_user_data_folder; 
      }


      using CookieResult = tasks::LoginTask::ResultWrapper;

      /// @brief Get a cookie for the CellarTracker website, if available
      /// 
      /// If we have saved credentials, this cookie will be retrieved in the background.
      /// 
      /// @return the requested cookie, or std::nullopt if cookie isn't available.
      /// 
      auto getCellarTrackerCookies() const -> const CookieResult&
      {
         return m_cookies;
      }


      /// @brief labelCacheFolder()
      /// @return the fully qualified path to the folder where label images are cached
      /// 
      auto labelCacheFolder() noexcept -> fs::path;

      /// @brief Get the current config object.
      ///
      /// Calling this will throw an exception if there's no default config. AFAIK the wxWidgets config store is 
      /// not thread-safe since multiple calls to SetPath() would be problematic. This should only be used from UI thread.
      ///
      auto getConfig(std::string_view initial_path = ScopedConfigPath::CONFIG_ROOT) noexcept(false) -> ScopedConfigPath;

      /// @brief Display a message box with an error description.
      ///
      /// If log_error is true, the exception will also be logged. source_loc is only used for logging.
      /// 
      void displayErrorMessage(const Error& err, bool log_error = true, std::source_location source_loc = std::source_location::current());
      void displayErrorMessage(const std::string& msg, bool log_error, const std::string& title = constants::ERROR_STR, std::source_location source_loc = std::source_location::current());

      /// @brief display a message box with informational text
      ///
      void displayInfoMessage(const std::string& msg, const std::string& title = constants::APP_NAME_SHORT);

   private:
      CookieResult       m_cookies{};
      MainFrame*         m_main_frame{};
      fs::path           m_user_data_folder{};
      std::stop_source   m_stop_source{};

      void OnCellarTrackerLogin(LoginEvent& event);

      void loginThread();
   };

}  // namespace ctb::app


wxDECLARE_APP(ctb::app::App);
