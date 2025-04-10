/*********************************************************************
 * @file       App.h
 *
 * @brief      Declaration for the App class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "app_constants.h"
#include "log.h"
#include "wx_helpers.h"

#include <ctb/TableProperty.h>
#include <wx/wx.h>




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

      /// @brief labelCacheFolder()
      /// @return the fully qualified path to the folder where label images are cached
      /// 
      auto labelCacheFolder() noexcept -> fs::path;

      /// @brief Get the current config object.
      ///
      /// Calling this will throw an exception  if there's no default config.
      ///
      auto getConfig() noexcept(false) -> ScopedConfigPath;

      /// @brief display a message box with an error description.
      ///
      void displayErrorMessage(const Error& err);
      void displayErrorMessage(const std::string& msg, const std::string& title = constants::ERROR_STR);

      /// @brief display a message box with inctb::formational text
      ///
      void displayInfoMessage(const std::string& msg, const std::string& title = constants::APP_NAME_SHORT);

   private:
      MainFrame* m_main_frame{};
      fs::path m_user_data_folder{ constants::CURRENT_DIRECTORY }; // safe default but should never actually be used since we set re-initialize it in OnInit()
   };

}  // namespace ctb::app


wxDECLARE_APP(ctb::app::App);
