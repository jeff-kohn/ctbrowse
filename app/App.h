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


   class LabelImageCache;   
   using LabelCachePtr = std::shared_ptr<LabelImageCache>;

   // we don't use enum class because then every time we need to pass an ID to wxObject,
   // we'd have to cast or use std::to_underlying and that's just an ugly waste of time 
   // with no benefit for this use-case.
   enum CmdId : uint16_t
   {
      CMD_FILE_OPEN = wxID_HIGHEST, 
      CMD_FILE_SAVE,
      CMD_FILE_DOWNLOAD_DATA,
      CMD_FILE_SETTINGS,
      CMD_EDIT_REFRESH_DATA,
      CMD_EDIT_CLEAR_FILTERS,
      CMD_FILTER_TREE_COLLAPSE_EXPAND,
      CMD_FILTER_TREE_DESELECT_ALL,
      CMD_FILTER_TREE_TOGGLE_CHECKED,
      CMD_FILTER_TREE_CLEAR_ALL,
      CMD_FILTER_TREE_COLLAPSE_ALL, 
      CND_FILTER_TREE_INVERT_SELECTION,
      CMD_COLLECTION_MY_CELLAR,
      CMD_COLLECTION_PENDING_WINE,
      CMD_COLLECTION_CONSUMED,
      CMD_COLLECTION_PURCHASED_WINE,
      CMD_COLLECTION_READY_TO_DRINK,
      CMD_COLLECTION_TAGGED_WINES,
      CMD_COLLECTION_TASTING_NOTES,
      CMD_ONLINE_WINE_DETAILS,
      CMD_ONLINE_SEARCH_VINTAGES,
      CMD_ONLINE_ACCEPT_PENDING,
      CMD_ONLINE_ADD_TASTING_NOTE,
      CMD_ONLINE_ADD_TO_CELLAR,
      CMD_ONLINE_DRINK_WINDOW,
      CMD_ONLINE_EDIT_ORDER,
      CMD_ONLINE_DRINK_REMOVE,
   };
   
   
   enum class AppFolder
   {
      Root,
      Defaults,
      Favorites,
      Labels,
      Tables
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
      auto getDataFolder(AppFolder folder) const noexcept -> fs::path
      {
         if (folder == AppFolder::Root)
            return m_user_data_folder; 

         auto path = ctb::format("{}/{}", m_user_data_folder.generic_string(), magic_enum::enum_name(folder));
         fs::create_directories(path);
         return path;
      }

      /// @brief Retrieve a pointer to main window that doesn't need dynamic_cast (or wx-equivalent).
      /// @return pointer to the main window. Will never be nullptr unless somehow called before OnInit().
      auto getMainWindow() const noexcept -> MainFrame*
      {
         return m_main_frame;
      }

      /// @brief labelCacheFolder()
      /// @return the fully qualified path to the folder where label images are cached
      auto getLabelCacheFolder() noexcept -> fs::path;
      void setLabelCacheFolder(const fs::path& cache_folder);

      auto getLabelCache() noexcept -> LabelCachePtr
      {
         return m_label_cache;
      }

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

      /// @brief display an info message to the user, using format()-style syntax for string building.
      template <typename... Args>
      void displayFormattedMessage(ctb::format_string<Args...> fmt_str, Args&&... args)
      {
         displayInfoMessage(ctb::vformat(fmt_str, ctb::make_format_args(args...)));
      }

   private:
      MainFrame*         m_main_frame{};
      fs::path           m_user_data_folder{};
      LabelCachePtr      m_label_cache{};
   };

}  // namespace ctb::app


wxDECLARE_APP(ctb::app::App);
