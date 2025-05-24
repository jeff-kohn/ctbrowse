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
#include <wx/app.h>


namespace ctb::app
{
   namespace fs = std::filesystem;


   /// @brief forward declare top-level window class so we don't have to add header dependency
   ///
   class MainFrame;

   // we don't use enum class because then every time we need to pass an ID to wxObject,
   // we'd have to cast or use std::to_underlying and that's just an ugly waste of time 
   // with no benefit for this use-case.
   enum CmdId : uint16_t
   {
      CMD_FILE_DOWNLOAD_DATA = wxID_HIGHEST,
      CMD_FILE_SETTINGS,
      CMD_COLLECTION_MY_CELLAR,
      CMD_COLLECTION_PENDING_WINE,
      CMD_WINE_ONLINE_DETAILS,
      CMD_WINE_ONLINE_VINTAGES,
      CMD_WINE_ONLINE_PRODUCER,
      CMD_WINE_ACCEPT_PENDING,
      CMD_WINE_EDIT_ORDER
   };

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
      auto userDataFolder() const noexcept  -> const fs::path& 
      {
         return m_user_data_folder; 
      }

      /// @brief Retrieve a pointer to main window that doesn't need dynamic_cast (or wx-equivalent).
      /// @return pointer to the main window. Will never be nullptr unless somehow called before OnInit().
      auto getMainWindow() const noexcept -> MainFrame*
      {
         return m_main_frame;
      }

      /// @brief labelCacheFolder()
      /// @return the fully qualified path to the folder where label images are cached
      auto labelCacheFolder() noexcept -> fs::path;

      /// @brief Get the current config object.
      ///
      /// Calling this will throw an exception if there's no default config. AFAIK the wxWidgets config store is 
      /// not thread-safe since multiple calls to SetPath() would be problematic. This should only be used from UI thread.
      auto getConfig(std::string_view initial_path = ScopedConfigPath::CONFIG_ROOT) noexcept(false) -> ScopedConfigPath;

      /// @brief Display a message box with an error description.
      ///
      /// If log_error is true, the exception will also be logged. source_loc is only used for logging.
      void displayErrorMessage(const Error& err, bool log_error = true, std::source_location source_loc = std::source_location::current());
      void displayErrorMessage(const std::string& msg, bool log_error, const std::string& title = constants::ERROR_STR, std::source_location source_loc = std::source_location::current());

      /// @brief display a message box with informational text
      void displayInfoMessage(const std::string& msg, const std::string& title = constants::APP_NAME_SHORT);

   private:
      MainFrame*         m_main_frame{};
      fs::path           m_user_data_folder{};
   };

}  // namespace ctb::app


wxDECLARE_APP(ctb::app::App);
