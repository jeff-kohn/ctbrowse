/*********************************************************************
 * @file       App.h
 *
 * @brief      Declaration for the App class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "app_constants.h"

#include <wx/wx.h>
#include <wx/confbase.h>

#include <filesystem>


namespace ctb::app
{
   namespace fs = std::filesystem;


   /// @brief forward declare top-level window class so we don't have to add header dependency
   ///
   class MainFrame;


   /// <summary>
   ///   app object for the OuraCharts applicatin.
   /// </summary>
   class App : public wxApp
   {
   public:
      App();

      /// @brief called by the framework on app startup, this is the place for program initialization 
      ///
      bool OnInit() override;


      /// @brief called by the framework on app shutdown, this is the place for resource cleanup and other shutdown tasks
      ///
      int OnExit() override;


      /// @brief  returns the path where the application stores data files.
      ///
      const fs::path& userDataFolder() const noexcept { return m_user_data_folder; }


      /// @brief Get the current config object.
      ///
      /// Calling this will throw an exception  instead of returning nullptr
      /// if there's no default config.
      ///
      wxConfigBase& getConfig() noexcept(false);
      const wxConfigBase& getConfig() const noexcept(false);


      /// @brief display a message box with an error description.
      ///
      void displayErrorMessage(const Error& err);
      void displayErrorMessage(const std::string& msg, const std::string& title = constants::ERROR_STR);

      /// @brief display a message box with informational text
      ///
      void displayInfoMessage(const std::string& msg, const std::string& title = constants::APP_NAME_SHORT);

   private:
      MainFrame* m_main_frame{};
      fs::path m_user_data_folder{ "." }; // safe default but should never actually be used since we set re-initialize it in OnInit()
   };

}  // namespace ctb::app


wxDECLARE_APP(ctb::app::App);
