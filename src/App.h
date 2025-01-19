/*********************************************************************
 * @file       App.h
 *
 * @brief      Declaration for the App class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "MainFrame.h"
#include "cts/constants.h"
#include "grids/CtGridTable.h"
#include <wx/wx.h>
#include <wx/confbase.h>

#include <filesystem>


namespace cts
{
   namespace fs = std::filesystem;

   /// <summary>
   ///   app object for the OuraCharts applicatin.
   /// </summary>
   class App : public wxApp
   {
   public:
      App();

      /// @brief called by the framework on app startup, this is the place for program initialization 
      bool OnInit() override;

      /// @brief called by the framework on app shutdown, this is the place for resource cleanup and other shutdown tasks
      int OnExit() override;

      /// @brief  returns the path where the application stores data files.
      const fs::path& userDataFolder() const noexcept { return m_user_data_folder; }

      /// @brief Get the current config object.
      ///
      /// Calling this will throw an exception  instead of returning nullptr
      /// if there's no default config.
      /// 
      wxConfigBase& getConfig() noexcept(false);
      const wxConfigBase& getConfig() const noexcept(false);

      CtGridTableMgr::GridTablePtr getGridTable(CtGridTableMgr::GridTable tbl);

   private:
      ui::MainFrame* m_main_frame{};
      cts::CtGridTableMgr m_grid_tables{};
      fs::path m_user_data_folder{ "." }; // safe default but should never actually be used since we set re-initialize it in OnInit()
   };

}  // namespace cts


wxDECLARE_APP(cts::App);
