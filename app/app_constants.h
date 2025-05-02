#pragma once

namespace ctb::constants
{
   inline constexpr const char* APP_NAME_LONG                     = "ctBrowse for Windows";
   inline constexpr const char* APP_NAME_SHORT                    = "ctBrowse";
   inline constexpr const char* APP_LABELS_SUBFOLDER              = "labels";
   inline constexpr const char* APP_DEFAULT_LOG_FOLDER            = "%LOCALAPPDATA%"; // safe default, but not used for windows app.


   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_LBL        = "&Download Data...";
   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_TIP        = "Download data from CellarTracker";
   inline constexpr const char* CMD_FILE_SETTINGS_LBL             = "&Settings...";
   inline constexpr const char* CMD_FILE_SETTINGS_TIP             = "Configure app settings";
   inline constexpr const char* CMD_VIEWS_WINE_LIST_LBL           = "&Wine List\tCtrl+W";
   inline constexpr const char* CMD_VIEWS_WINE_LIST_TIP           = "View Personal Wine List";
   inline constexpr const char* CMD_VIEWS_RESIZE_COLS_LBL         = "&Resize Data Columns";
   inline constexpr const char* CMD_VIEWS_RESIZE_COLS_TIP         = "Resize the data columns to fit the data";

   inline constexpr const char* CONFIG_PATH_PREFERENCES           = "/Preferences";
   inline constexpr const char* CONFIG_PATH_PREFERENCE_DATASYNC   = "/Preferences/DataSync";

   inline constexpr const char* CONFIG_VALUE_DEFAULT_SYNC_TABLES  = "DefaultSyncTables";
   inline constexpr const char* CONFIG_VALUE_SYNC_ON_STARTUP      = "SyncOnStartup";
   inline constexpr const char* CONFIG_VALUE_LABEL_CACHE_DIR      = "LabelCacheDir";
   inline constexpr const char* CONFIG_PATH_GRID_OPTIONS          = "/Preferences/GridOptions";
   inline constexpr const char* CONFIG_VALUE_DEFAULT_IN_STOCK_ONLY= "DefaultInStockOnly";

   inline constexpr const char* ERROR_WINDOW_CREATION_FAILED      = "Window creation failed. You may need to restart the app.";

   inline constexpr const char* FILTER_APPELATION                 = "Appellation";
   inline constexpr const char* FILTER_COUNTRY                    = "Country";
   inline constexpr const char* FILTER_REGION                     = "Region";
   inline constexpr const char* FILTER_VARIETAL                   = "Varietal";

   inline constexpr const char* FMT_CREDENTIALDLG_PROMPT_MSG      = "Enter the username and password for {}:";
   inline constexpr const char* FMT_CREDENTIALDLG_REPROMPT_MSG    = "Authentication failed. Re-enter username and password for {}:";
   inline constexpr const char* CREDENTIALDLG_LBL_USERNAME        = "&Username:";
   inline constexpr const char* CREDENTIALDLG_LBL_PASSWORD        = "&Password:";
   inline constexpr const char* CREDENTIALDLG_LBL_SAVE            = "&Save Credential";
   inline constexpr const char* FMT_CREDENTIALDLG_LBL_TITLE       = "{} Login";
   
   inline constexpr const char* FMT_LBL_FILTERED_ROWS             = "Filtered Rows: {}";
   inline constexpr const char* FMT_LBL_FILTERS_SELECTED          = "{}  ({} selected)";
   inline constexpr const char* FMT_LBL_TOTAL_ROWS                = "Total Rows: {}";
   inline constexpr const char* FMT_STATUS_FILE_DOWNLOADED        = "Successfully downloaded file '{}'.";
   inline constexpr const char* FMT_STATUS_FILE_DOWNLOADING       = "Downloading file '{}'...";
   inline constexpr const char* FMT_TITLE_TYPED_ERROR             = "{} Error";
   inline constexpr const char* FMT_LABEL_IMAGE_FILENAME          = "{}-{}.jpg";

   inline constexpr const char* INFO_MSG_NO_MATCHING_ROWS         = "No rows matched the search text.";
   inline constexpr const char* ERROR_USER_CANCELED               = "User canceled operation.";
   inline constexpr const char* NO_SCORE                          = " --";
   
   // column labels
   inline constexpr const char* COL_CT_SCORE                      = "CT Score";
   inline constexpr const char* COL_LOCALE                        = "Region and Appellation";
   inline constexpr const char* COL_MY_SCORE                      = "My Score";
   inline constexpr const char* COL_QTY                           = "Qty";
   inline constexpr const char* COL_WINE                          = "Wine";


   // DetailsPanel labels
   inline constexpr const char* LBL_APPELLATION                   = "Appellation:";
   inline constexpr const char* LBL_AUCTION_PRICE                 = "Auction Value:";
   inline constexpr const char* LBL_CHECK_IN_STOCK_ONLY           = "Only Show In-Stock Inventory";
   inline constexpr const char* LBL_COUNTRY                       = "Country:";
   inline constexpr const char* LBL_CT_PRICE                      = "Community Avg:";
   inline constexpr const char* LBL_CT_SCORE                      = "CT Score:";
   inline constexpr const char* LBL_DRINK_WINDOW                  = "Drink Window:";
   inline constexpr const char* LBL_FILTER_OPTIONS                = "Filter Options";
   inline constexpr const char* LBL_MENU_VIEW                     = "&View";
   inline constexpr const char* LBL_MY_SCORE                      = "My Score:";
   inline constexpr const char* LBL_MY_PRICE                      = "My Price:";
   inline constexpr const char* LBL_REGION                        = "Region:";
   inline constexpr const char* LBL_REQUIRE_MIN_SCORE             = "Only Show Scores Above:";
   inline constexpr const char* LBL_SCORES                        = "Scores";
   inline constexpr const char* LBL_SEARCH                        = "&Search";
   inline constexpr const char* LBL_SORT_ASCENDING                = "Ascending";
   inline constexpr const char* LBL_SORT_DESCENDING               = "Descending";
   inline constexpr const char* LBL_SORT_OPTIONS                  = "Sort Options";
   inline constexpr const char* LBL_SUB_REGION                    = "Subregion:";
   inline constexpr const char* LBL_VALUATION                     = "Valuation";
   inline constexpr const char* LBL_VARIETAL                      = "Master Varietal:";
   inline constexpr const char* LBL_VINTAGE                       = "Vintage:";


   // Detail property names
   inline constexpr const char* DETAIL_PROP_APPELLATION           = "Appellation";
   inline constexpr const char* DETAIL_PROP_AUCTION_VALUE         = "AuctionPrice";
   inline constexpr const char* DETAIL_PROP_COMMUNITY_PRICE       = "CtPrice";
   inline constexpr const char* DETAIL_PROP_COUNTRY               = "Country";
   inline constexpr const char* DETAIL_PROP_CT_SCORE              = "CTScore";
   inline constexpr const char* DETAIL_PROP_DRINK_START           = "BeginConsume";
   inline constexpr const char* DETAIL_PROP_DRINK_END             = "EndConsume";
   inline constexpr const char* DETAIL_PROP_LOCALE                = "Locale";
   inline constexpr const char* DETAIL_PROP_MY_PRICE              = "Price";
   inline constexpr const char* DETAIL_PROP_MY_SCORE              = "MYScore";
   inline constexpr const char* DETAIL_PROP_REGION                = "Region";
   inline constexpr const char* DETAIL_PROP_SUB_REGION            = "SubRegion";
   inline constexpr const char* DETAIL_PROP_VARIETAL              = "MasterVarietal";
   inline constexpr const char* DETAIL_PROP_VINTAGE               = "Vintage";
   inline constexpr const char* DETAIL_PROP_WINE_NAME             = "WineName";
   inline constexpr const char* DETAIL_PROP_WINE_ID               = "iWineId";
   inline constexpr const char* DETAIL_VIEW_ONLINE_TITLE          = "View Online at CellarTracker.com";
   inline constexpr const char* DETAIL_VIEW_ONLINE_NOTE           = "(Opens in default browser)";

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
   inline constexpr const char* SORT_OPTION_SCORE_MY              = "My Score";
   inline constexpr const char* SORT_OPTION_SCORE_CT              = "CellarTracker Score";
   inline constexpr const char* SORT_OPTION_MY_VALUE              = "My Value";
   inline constexpr const char* SORT_OPTION_VARIETAL_WINE         = "Varietal, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_VINTAGE_WINE          = "Vintage and Wine";
   inline constexpr const char* SORT_OPTION_WINE_VINTAGE          = "Wine and Vintage";

   inline constexpr const char* STATUS_DOWNLOAD_CANCELED          = "Download operation canceled.";
   inline constexpr const char* STATUS_DOWNLOAD_COMPLETE          = "Download operation completed.";
   inline constexpr const char* STATUS_DOWNLOAD_FAILED            = "Download operation failed.";
   inline constexpr const char* TITLE_DOWNLOAD_DATA               = "Download Data";

   inline constexpr double FILTER_SCORE_DEFAULT =  90.0;
   inline constexpr double FILTER_SCORE_INCR    =   0.1;
   inline constexpr double FILTER_SCORE_MIN     =    50;
   inline constexpr double FILTER_SCORE_MAX     = 100.0;
   inline constexpr int FILTER_SCORE_DIGITS     =     1;
   inline constexpr int WX_UNSPECIFIED_VALUE    =    -1;
   inline constexpr bool CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT     = false;

   // app-specific error messages.
   inline constexpr const char* ERROR_STR_LABEL_CACHE_SHUT_DOWN = "Label cache object is shutting down.";
   inline constexpr const char* ERROR_STR_NULL_POINTER          = "NULL pointer argument not allowed.";
   inline constexpr const char* ERROR_STR_NO_CONFIG_STORE       = "No configuration store available.";
   inline constexpr const char* CT_COOKIE_SECRET_NAME           = "CTSession";


}  // namespace ctb::constants