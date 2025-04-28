/*********************************************************************
 * @file       App.h
 *
 * @brief      Declaration for the App class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "app_constants.h"
#include "wx_helpers.h"

#include <ctb/log.h>
#include <ctb/TableProperty.h>

#include <cpr/cookies.h>
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



      /// @brief Retrieve the cookies for use with CT website.
      /// 
      /// Connects to the CT website using saved or user-prompted credential and retrieves user Cookie
      /// for connection to and interacting with CT website.
      /// 
      /// @param prompt_for_cred - if true and the saved cred is not available, user will be prompted for 
      ///        username and password.
      /// 
      /// @return the requested cookies if successful, a ctb::Error if unsuccessful.
      /// 
      //auto getCellarTrackerCookies(bool prompt_for_cred) -> CookieResult;


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

      /// @brief display a message box with inctb::formational text
      ///
      void displayInfoMessage(const std::string& msg, const std::string& title = constants::APP_NAME_SHORT);

   private:
      using MaybeCookies  = std::optional<cpr::Cookies>;
      MaybeCookies m_ct_cookies{};
      MainFrame* m_main_frame{};
      fs::path m_user_data_folder{ constants::CURRENT_DIRECTORY }; // safe default but should never actually be used since we set re-initialize it in OnInit()
   };

}  // namespace ctb::app


wxDECLARE_APP(ctb::app::App);
