#pragma once

namespace ctb::constants
{
   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_LBL        = "&Download Data...";
   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_TIP        = "Download data from CellarTracker";
   inline constexpr const char* CMD_FILE_SETTINGS_LBL             = "&Settings...";
   inline constexpr const char* CMD_FILE_SETTINGS_TIP             = "Configure app settings";
   inline constexpr const char* CMD_VIEWS_WINE_LIST_LBL           = "&Wine List\tCtrl+W";
   inline constexpr const char* CMD_VIEWS_WINE_LIST_TIP           = "View Personal Wine List";

   inline constexpr const char* CONFIG_PATH_SYNC                  = "/Preferences/DataSync";
   inline constexpr const char* CONFIG_VALUE_DEFAULT_SYNC_TABLES  = "DefaultSyncTables";
   inline constexpr const char* CONFIG_VALUE_SYNC_ON_STARTUP      = "SyncOnStartup";

   inline constexpr const char* ERROR_WINDOW_CREATION_FAILED      = "Window creation failed. You may need to restart the app.";

   inline constexpr const char* FILTER_APPELATION                 = "Appellation";
   inline constexpr const char* FILTER_COUNTRY                    = "Country";
   inline constexpr const char* FILTER_REGION                     = "Region";
   inline constexpr const char* FILTER_VARIETAL                   = "Varietal";

   inline constexpr const char* FMT_LBL_FILTERED_ROWS             = "Filtered Rows: {}";
   inline constexpr const char* FMT_LBL_FILTERS_SELECTED          = "{}  ({} selected)";
   inline constexpr const char* FMT_LBL_TOTAL_ROWS                = "Total Rows: {}";
   inline constexpr const char* FMT_STATUS_FILE_DOWNLOADED        = "Successfully downloaded file '{}'.";
   inline constexpr const char* FMT_STATUS_FILE_DOWNLOADING       = "Downloading file '{}'...";
   inline constexpr const char* FMT_TITLE_TYPED_ERROR             = "Error ({})";

   inline constexpr const char* INFO_MSG_NO_MATCHING_ROWS         = "No rows matched the search text.";

   inline constexpr const char* LBL_CT_SCORE                      = "CT Score";
   inline constexpr const char* LBL_DRINK_BY                      = "Drink By";
   inline constexpr const char* LBL_FILTER_OPTIONS                = "Filter Options";
   inline constexpr const char* LBL_LOCALE                        = "Region and Appellation";
   inline constexpr const char* LBL_MENU_VIEWS                    = "&Views";
   inline constexpr const char* LBL_MY_PRICE                      = "My Price";
   inline constexpr const char* LBL_MY_SCORE                      = "My Score";
   inline constexpr const char* LBL_QTY                           = "Qty";
   inline constexpr const char* LBL_SEARCH                        = "&Search";
   inline constexpr const char* LBL_SORT_ASCENDING                = "Ascending";
   inline constexpr const char* LBL_SORT_DESCENDING               = "Descending";
   inline constexpr const char* LBL_SORT_OPTIONS                  = "Sort Options";
   inline constexpr const char* LBL_VARIETAL                      = "Varietal";
   inline constexpr const char* LBL_WINE                          = "Wine";

   inline constexpr const char* RES_NAME_ICON_PRODUCT             = "IDR_PRODUCT_ICON";
   inline constexpr const char* RES_NAME_MAINFRAME                = "MAINFRAME";
   inline constexpr const char* RES_NAME_TREE_CHECKED_IMG         = "TREE_CHECKED_IMG";
   inline constexpr const char* RES_NAME_TREE_FILTER_IMG          = "TREE_FILTER_IMG";
   inline constexpr const char* RES_NAME_TREE_UNCHECKED_IMG       = "TREE_UNCHECKED_IMG";

   inline constexpr const char* SORT_OPTION_APPELATION_WINE       = "Appellation, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_COUNTRY_APPELATION    = "Country, Appellation and Wine";
   inline constexpr const char* SORT_OPTION_LOCALE_WINE           = "Country, Region, and Appellation";
   inline constexpr const char* SORT_OPTION_PRODUCER_WINE_NAME    = "Producer, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_REGION_WINE           = "Region, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_VARIETAL_WINE         = "Varietal, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_VINTAGE_WINE          = "Vintage and Wine";
   inline constexpr const char* SORT_OPTION_WINE_VINTAGE          = "Wine and Vintage";

   inline constexpr const char* STATUS_DOWNLOAD_CANCELED          = "Download operation canceled.";
   inline constexpr const char* STATUS_DOWNLOAD_COMPLETE          = "Download operation completed.";
   inline constexpr const char* STATUS_DOWNLOAD_FAILED            = "Download operation failed.";
   inline constexpr const char* TITLE_DOWNLOAD_DATA               = "Download Data";

   inline constexpr int WX_UNSPECIFIED_VALUE = -1;

}  // namespace ctb::app::constants