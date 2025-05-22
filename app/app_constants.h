#pragma once

namespace ctb::constants
{
   inline constexpr const char* APP_NAME_LONG                     = "ctBrowse for Windows";
   inline constexpr const char* APP_NAME_SHORT                    = "ctBrowse";
   inline constexpr const char* APP_LABELS_SUBFOLDER              = "labels";
   inline constexpr const char* APP_DEFAULT_LOG_FOLDER            = "%LOCALAPPDATA%"; // safe default, but not used for windows app.


   inline constexpr const char* LBL_MENU_DATA                     = "&Datasets";
   inline constexpr const char* LBL_MENU_WINE                     = "&Online";
   inline constexpr const char* LBL_MENU_VIEW                     = "&View";
   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_LBL        = "&Download Data...";
   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_TIP        = "Download data from CellarTracker";
   inline constexpr const char* CMD_FILE_SETTINGS_LBL             = "&Settings...";
   inline constexpr const char* CMD_FILE_SETTINGS_TIP             = "Configure app settings";
   inline constexpr const char* CMD_DATA_WINE_LIST_LBL            = "&My Wine Collection\tCtrl+M";
   inline constexpr const char* CMD_DATA_WINE_LIST_TIP            = "View Personal Wine Collection";
   inline constexpr const char* CMD_DATA_PENDING_WINE_LBL         = "&Pending Wine Orders\tCtrl+P";
   inline constexpr const char* CMD_DATA_PENDING_WINE_TIP         = "View Pending Wine Deliveries";
   inline constexpr const char* CMD_VIEW_AUTOLAYOUT_COLS_LBL      = "&Resize List Columns";
   inline constexpr const char* CMD_VIEW_AUTOLAYOUT_COLS_TIP      = "Resize the list columns to fit the data";
   inline constexpr const char* CMD_WINE_ONLINE_DETAILS_LBL       = "View &Details Online";
   inline constexpr const char* CMD_WINE_ONLINE_DETAILS_TIP       = "View the current wine's details on website";
   inline constexpr const char* CMD_WINE_ONLINE_VINTAGES_LBL      = "View &Vintages Online";
   inline constexpr const char* CMD_WINE_ONLINE_VINTAGES_TIP      = "View all vintages of the current wine on website";
   inline constexpr const char* CMD_WINE_ONLINE_PRODUCER_LBL      = "View &Producer Online";
   inline constexpr const char* CMD_WINE_ONLINE_PRODUCER_TIP      = "View Producer of the current wine on website";
   inline constexpr const char* CMD_WINE_ONLINE_ACCEPT_WINE_LBL   = "&Accept Delivery Online";
   inline constexpr const char* CMD_WINE_ONLINE_ACCEPT_WINE_TIP   = "Mark the selected wine as delivered on website";

   inline constexpr const char* CONFIG_PATH_PREFERENCES           = "/Preferences";
   inline constexpr const char* CONFIG_PATH_PREFERENCE_DATASYNC   = "/Preferences/DataSync";

   inline constexpr const char* CONFIG_VALUE_DEFAULT_SYNC_TABLES  = "DefaultSyncTables";
   inline constexpr const char* CONFIG_VALUE_SYNC_ON_STARTUP      = "SyncOnStartup";
   inline constexpr const char* CONFIG_VALUE_LABEL_CACHE_DIR      = "LabelCacheDir";
   inline constexpr const char* CONFIG_PATH_GRID_OPTIONS          = "/Preferences/GridOptions";
   inline constexpr const char* CONFIG_VALUE_DEFAULT_IN_STOCK_ONLY= "DefaultInStockOnly";

   inline constexpr const char* ERROR_WINDOW_CREATION_FAILED      = "Window creation failed. You may need to restart the app.";

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
   


   // DetailsPanel labels
   inline constexpr const char* LBL_APPELLATION                   = "Appellation:";
   inline constexpr const char* LBL_AUCTION_PRICE                 = "Auction Value:";
   inline constexpr const char* LBL_CHECK_IN_STOCK_ONLY           = "Only Show In-Stock Inventory";
   inline constexpr const char* LBL_COUNTRY                       = "Country:";
   inline constexpr const char* LBL_CT_PRICE                      = "Community Avg:";
   inline constexpr const char* LBL_CT_SCORE                      = "CT Score:";
   inline constexpr const char* LBL_DRINK_WINDOW                  = "Drink Window:";
   inline constexpr const char* LBL_FILTER_OPTIONS                = "Filter Options";
   inline constexpr const char* LBL_MY_SCORE                      = "My Score:";
   inline constexpr const char* LBL_MY_PRICE                      = "My Price:";
   inline constexpr const char* LBL_DELIVERY_DATE                 = "Delivery Date:";
   inline constexpr const char* LBL_ORDER_DATE                    = "Order Date:";
   inline constexpr const char* LBL_ORDER_DETAILS                 = "Order Details";
   inline constexpr const char* LBL_ORDER_NUMBER                  = "Order Number:";
   inline constexpr const char* LBL_QTY_ORDERED                   = "Qty Ordered:";
   inline constexpr const char* LBL_REGION                        = "Region:";
   inline constexpr const char* LBL_REQUIRE_MIN_SCORE             = "Only Show Scores Above:";
   inline constexpr const char* LBL_SCORES                        = "Scores";
   inline constexpr const char* LBL_SEARCH                        = "&Search";
   inline constexpr const char* LBL_SORT_ASCENDING                = "Ascending";
   inline constexpr const char* LBL_SORT_DESCENDING               = "Descending";
   inline constexpr const char* LBL_SORT_OPTIONS                  = "Sort Options";
   inline constexpr const char* LBL_STORE_NAME                    = "Store:";
   inline constexpr const char* LBL_SUB_REGION                    = "Subregion:";
   inline constexpr const char* LBL_VALUATION                     = "Valuation";
   inline constexpr const char* LBL_VARIETAL                      = "Master Varietal:";
   inline constexpr const char* LBL_VINTAGE                       = "Vintage:";

   inline constexpr const char* DETAIL_VIEW_ONLINE_BTN_TITLE      = "View Wine on CellarTracker.com";
   inline constexpr const char* DETAIL_VIEW_ONLINE_BTN_NOTE       = "(Opens in default browser)";

   inline constexpr const char* DETAIL_ACCEPT_PENDING_BTN_TITLE   = "Accept Delivery on CellarTracker.com";
   inline constexpr const char* DETAIL_ACCEPT_PENDING_BTN_NOTE    = "(Opens in default browser)";

   inline constexpr const char* RES_NAME_ICON_PRODUCT             = "IDR_PRODUCT_ICON";
   inline constexpr const char* RES_NAME_MAINFRAME                = "MAINFRAME";
   inline constexpr const char* RES_NAME_TREE_CHECKED_IMG         = "TREE_CHECKED_IMG";
   inline constexpr const char* RES_NAME_TREE_FILTER_IMG          = "TREE_FILTER_IMG";
   inline constexpr const char* RES_NAME_TREE_UNCHECKED_IMG       = "TREE_UNCHECKED_IMG";

   inline constexpr const char* STATUS_DOWNLOAD_CANCELED          = "Download operation canceled.";
   inline constexpr const char* STATUS_DOWNLOAD_COMPLETE          = "Download operation completed.";
   inline constexpr const char* STATUS_DOWNLOAD_FAILED            = "Download operation failed.";
   inline constexpr const char* TITLE_DOWNLOAD_DATA               = "Download Data";

   inline constexpr int WX_UNSPECIFIED_VALUE    =    -1;
   inline constexpr bool CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT     = false;

   // app-specific error messages.
   inline constexpr const char* ERROR_STR_LABEL_CACHE_SHUT_DOWN = "Label cache object is shutting down.";
   inline constexpr const char* ERROR_STR_NULL_POINTER          = "NULL pointer argument not allowed.";
   inline constexpr const char* ERROR_STR_NO_CONFIG_STORE       = "No configuration store available.";
   inline constexpr const char* CT_COOKIE_SECRET_NAME           = "CTSession";


}  // namespace ctb::constants