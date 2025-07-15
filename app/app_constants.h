#pragma once

namespace ctb::constants
{
   inline constexpr const char* APP_NAME_LONG                     = "ctBrowse for Windows";
   inline constexpr const char* APP_NAME_SHORT                    = "ctBrowse";
   
   inline constexpr const char* FILE_COLLECTION_CTBC_FILTER        = "ctBrowse Collection (*.ctbc)|*.ctbc";
   inline constexpr const char* FILE_OPEN_COLLECTION_FILTER        = "Select Saved ctBrowse Collection:";

   inline constexpr const char* LBL_MENU_COLLECTION                = "&Collection";
   inline constexpr const char* LBL_MENU_WINE                      = "&Online";
   inline constexpr const char* LBL_MENU_VIEW                      = "&View";

   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_LBL         = "&Download Data...";
   inline constexpr const char* CMD_FILE_DOWNLOAD_DATA_TIP         = "Download data from CellarTracker";
   inline constexpr const char* CMD_FILE_OPEN_LBL                  = "&Open Collection...";
   inline constexpr const char* CMD_FILE_OPEN_TIP                  = "Open saved collection settings";
   inline constexpr const char* CMD_FILE_SAVE_LBL                  = "&Save Collection...";
   inline constexpr const char* CMD_FILE_SAVE_TIP                  = "Save the current  collection settings to file";
   inline constexpr const char* CMD_FILE_SETTINGS_LBL              = "&Settings...";
   inline constexpr const char* CMD_FILE_SETTINGS_TIP              = "Configure app settings";
   inline constexpr const char* CMD_EDIT_REFRESH_DATA_LBL          = "&Refresh Collection Data\tCtrl+R";
   inline constexpr const char* CMD_EDIT_REFRESH_DATA_TIP          = "Reload the current collection's data";
   inline constexpr const char* CMD_EDIT_CLEAR_FILTERS_LBL         = "Clear Filters";
   inline constexpr const char* CMD_EDIT_CLEAR_FILTERS_TIP         = "Remove all active data filters.";
   inline constexpr const char* CMD_COLLECTION_CONSUMED_LBL        = "Wine &Consumed";
   inline constexpr const char* CMD_COLLECTION_CONSUMED_TIP        = "View Wine Consumption History";
   inline constexpr const char* CMD_COLLECTION_MY_CELLAR_LBL       = "Wine in &My Cellar\tCtrl+M";
   inline constexpr const char* CMD_COLLECTION_MY_CELLAR_TIP       = "View Personal Wine Collection";
   inline constexpr const char* CMD_COLLECTION_PENDING_WINE_LBL    = "Wine &Pending Delivery\tCtrl+P";
   inline constexpr const char* CMD_COLLECTION_PENDING_WINE_TIP    = "View Pending Wine Deliveries";
   inline constexpr const char* CMD_COLLECTION_READY_TO_DRINK_LBL  = "Wine &Ready to Drink\tCtrl+D";
   inline constexpr const char* CMD_COLLECTION_READY_TO_DRINK_TIP  = "View Drinking Availability for Wines";
   inline constexpr const char* CMD_ONLINE_WINE_DETAILS_LBL        = "View &Wine Details";
   inline constexpr const char* CMD_ONLINE_WINE_DETAILS_TIP        = "View the current wine's details on CellarTracker";
   inline constexpr const char* CMD_ONLINE_SEARCH_VINTAGES_LBL     = "Search All &Vintages";
   inline constexpr const char* CMD_ONLINE_SEARCH_VINTAGES_TIP     = "View all vintages of the current wine on CellarTracker";
   inline constexpr const char* CMD_ONLINE_ADD_TASTING_NOTE_LBL    = "Add &Tasting Note";
   inline constexpr const char* CMD_ONLINE_ADD_TASTING_NOTE_TIP    = "Add Tasting Note for current wine on CellarTracker";
   inline constexpr const char* CMD_ONLINE_DRINK_WINDOW_LBL        = "Edit Drin&k Window";
   inline constexpr const char* CMD_ONLINE_DRINK_WINDOW_TIP        = "Edit drink window of current wine on CellarTracker";
   inline constexpr const char* CMD_ONLINE_ADD_TO_CELLAR_LBL       = "&Add to My Cellar";
   inline constexpr const char* CMD_ONLINE_ADD_TO_CELLAR_TIP       = "Add more of current wine to inventory on CellarTracker";
   inline constexpr const char* CMD_ONLINE_ACCEPT_PENDING_LBL      = "Accept &Delivery";
   inline constexpr const char* CMD_ONLINE_ACCEPT_PENDING_TIP      = "Mark the selected wine as delivered on CellarTracker";
   inline constexpr const char* CMD_ONLINE_EDIT_ORDER_LBL          = "&Edit Order Details";
   inline constexpr const char* CMD_ONLINE_EDIT_ORDER_TIP          = "Edit the selected purchase order on CellarTracker";
   inline constexpr const char* CMD_ONLINE_DRINK_REMOVE_LBL        = "Drink/&Remove";
   inline constexpr const char* CMD_ONLINE_DRINK_REMOVE_TIP        = "Mark a bottle as drunk/removed on CellarTracker";
   inline constexpr const char* CMD_ONLINE_VIEW_ON_CT_LBL          = "View on CellarTracker";
   inline constexpr const char* CMD_ONLINE_VIEW_ON_CT_TIP          = "View the Wine Details page for the selected wine on CellarTracker";
   inline constexpr const char* CMD_FILTER_TREE_CHECK_FILTER_LBL   = "&Apply Filter";
   inline constexpr const char* CMD_FILTER_TREE_CHECK_FILTER_TIP   = "Add the current value to the match values for the filter.";
   inline constexpr const char* CMD_FILTER_TREE_UNCHECK_FILTER_LBL = "&Disable Filter";
   inline constexpr const char* CMD_FILTER_TREE_UNCHECK_FILTER_TIP = "Remove the current value from the match values or the filter.";
   inline constexpr const char* CMD_FILTER_TREE_COLLAPSE_LBL       = "&Collapse";
   inline constexpr const char* CMD_FILTER_TREE_COLLAPSE_TIP       = "Collapse the current filter node.";
   inline constexpr const char* CMD_FILTER_TREE_COLLAPSE_ALL_LBL   = "&Collapse All";
   inline constexpr const char* CMD_FILTER_TREE_COLLAPSE_ALL_TIP   = "Collapse all filter nodes.";
   inline constexpr const char* CMD_FILTER_TREE_CLEAR_ALL_LBL      = "&Remove All Filters";
   inline constexpr const char* CMD_FILTER_TREE_CLEAR_ALL_TIP      = "Remove all multi-value filters.";
   inline constexpr const char* CMD_FILTER_TREE_DESELECT_ALL_LBL   = "&Deselect All";
   inline constexpr const char* CMD_FILTER_TREE_DESELECT_ALL_TIP   = "Deselect all values for the current filter node.";
   inline constexpr const char* CMD_FILTER_TREE_EXPAND_LBL         = "&Expand";
   inline constexpr const char* CMD_FILTER_TREE_EXPAND_TIP         = "Expand the current filter node.";

   inline constexpr const char* CONFIG_PATH_PREFERENCES            = "/Preferences";
   inline constexpr const char* CONFIG_PATH_PREFERENCE_DATASYNC    = "/Preferences/DataSync";
 
   inline constexpr const char* CONFIG_VALUE_DEFAULT_SYNC_TABLES   = "DefaultSyncTables";
   inline constexpr const char* CONFIG_VALUE_SYNC_ON_STARTUP       = "SyncOnStartup";
   inline constexpr const char* CONFIG_VALUE_LABEL_CACHE_DIR       = "LabelCacheDir";
   inline constexpr const char* CONFIG_PATH_GRID_OPTIONS           = "/Preferences/GridOptions";
   inline constexpr const char* CONFIG_VALUE_DEFAULT_IN_STOCK_ONLY = "DefaultInStockOnly";
 
   inline constexpr const char* ERROR_WINDOW_CREATION_FAILED       = "Window creation failed. You may need to restart the app.";

   inline constexpr const char* FMT_CREDENTIALDLG_PROMPT_MSG      = "Enter the username and password for {}:";
   inline constexpr const char* FMT_CREDENTIALDLG_REPROMPT_MSG    = "Authentication failed. Re-enter username and password for {}:";
   inline constexpr const char* CREDENTIALDLG_LBL_USERNAME        = "&Username:";
   inline constexpr const char* CREDENTIALDLG_LBL_PASSWORD        = "&Password:";
   inline constexpr const char* CREDENTIALDLG_LBL_SAVE            = "&Save Credential";
   inline constexpr const char* FMT_CREDENTIALDLG_LBL_TITLE       = "{} Login";

   inline constexpr const char* FMT_DEFAULT_OPTIONS_SAVED_MSG     = "New default options saved for collection '{}'";
   inline constexpr const char* FMT_LBL_FILTERS_SELECTED          = "{}  ({} selected)";
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
   //inline constexpr const char* LBL_BTN_SAVE_DEFAULT              = "&Save as Default";
   //inline constexpr const char* LBL_BTN_SAVE_DEFAULT_TIP          = "Save the current filter options as the default for this collection.";
   //inline constexpr const char* LBL_BTN_CLEAR_FILTERS             = "&Clear Filters";
   //inline constexpr const char* LBL_BTN_CLEAR_FILTERS_TIP         = "Clear all filters and show all records.";
   inline constexpr const char* LBL_CHECK_IN_STOCK_ONLY           = "Only Show In-Stock Wines";
   inline constexpr const char* LBL_CHECK_MIN_SCORE               = "Minimum Score:";
   inline constexpr const char* LBL_CHECK_READY_TO_DRINK          = "Only Show 'Ready' Wines";
   inline constexpr const char* LBL_CONSUME_DATE                  = "Consumed On:";
   inline constexpr const char* LBL_CONSUME_REASON                = "Reason:";
   inline constexpr const char* LBL_COUNTRY                       = "Country:";
   inline constexpr const char* LBL_CT_PRICE                      = "Community Avg:";
   inline constexpr const char* LBL_CT_SCORE                      = "CT Score:";
   inline constexpr const char* LBL_DRINK_WINDOW                  = "Drink Window:";
   inline constexpr const char* LBL_DRINK_WINDOW_MY               = "My Drink Window:";
   inline constexpr const char* LBL_DRINK_WINDOW_CT               = "CT Drink Window:";
   inline constexpr const char* LBL_FILTER_OPTIONS                = "Filter Options";
   inline constexpr const char* LBL_LOCATION                      = "Location:";
   inline constexpr const char* LBL_MY_SCORE                      = "My Score:";
   inline constexpr const char* LBL_MY_PRICE                      = "My Price:";
   inline constexpr const char* LBL_DELIVERY_DATE                 = "Delivery Date:";
   inline constexpr const char* LBL_ORDER_DATE                    = "Order Date:";
   inline constexpr const char* LBL_ORDER_DETAILS                 = "Order Details";
   inline constexpr const char* LBL_ORDER_NUMBER                  = "Order Number:";
   inline constexpr const char* LBL_QTY_ORDERED                   = "Qty Ordered:";
   inline constexpr const char* LBL_REGION                        = "Region:";
   inline constexpr const char* LBL_SCORES                        = "Scores";
   inline constexpr const char* LBL_SEARCH                        = "&Search";
   inline constexpr const char* LBL_SIZE                          = "Size:";
   inline constexpr const char* LBL_SORT_ASCENDING                = "Ascending";
   inline constexpr const char* LBL_SORT_DESCENDING               = "Descending";
   inline constexpr const char* LBL_SORT_OPTIONS                  = "Sort Options";
   inline constexpr const char* LBL_STORE_NAME                    = "Store:";
   inline constexpr const char* LBL_SUB_REGION                    = "Subregion:";
   inline constexpr const char* LBL_VALUATION                     = "Valuation";
   inline constexpr const char* LBL_VARIETAL                      = "Master Varietal:";
   inline constexpr const char* LBL_VINTAGE                       = "Vintage:";

   inline constexpr const char* DETAILS_CMD_LINK_NOTE             = "(Opens in default browser)";
   inline constexpr const char* DETAILS_CMD_LINK_WINE_DETAILS     = "View Wine on CellarTracker.com";
   inline constexpr const char* DETAILS_CMD_LINK_ACCEPT_PENDING   = "Accept Delivery on CellarTracker.com";
   inline constexpr const char* DETAILS_CMD_LINK_DRINK_REMOVE     = "Drink/Remove on CellarTracker.com";

   inline constexpr const char* RES_NAME_ICON_PRODUCT             = "IDR_PRODUCT_ICON";
   inline constexpr const char* RES_NAME_MAINFRAME                = "MAINFRAME";
   inline constexpr const char* RES_NAME_TREE_CHECKED_IMG         = "TREE_CHECKED_IMG";
   inline constexpr const char* RES_NAME_TREE_FILTER_IMG          = "TREE_FILTER_IMG";
   inline constexpr const char* RES_NAME_TREE_UNCHECKED_IMG       = "TREE_UNCHECKED_IMG";

   inline constexpr const char* STATUS_DOWNLOAD_CANCELED          = "Download operation canceled.";
   inline constexpr const char* STATUS_DOWNLOAD_COMPLETE          = "Download operation completed.";
   inline constexpr const char* STATUS_DOWNLOAD_FAILED            = "Download operation failed.";
   inline constexpr const char* TITLE_DOWNLOAD_DATA               = "Download Data";

   inline constexpr int  WX_UNSPECIFIED_VALUE                     = -1;
   inline constexpr bool CONFIG_VALUE_IN_STOCK_FILTER_DEFAULT     = true;

   // app-specific error messages.
   inline constexpr const char* ERROR_STR_LABEL_CACHE_SHUT_DOWN = "Label cache object is shutting down.";
   inline constexpr const char* ERROR_STR_NO_CONFIG_STORE       = "No configuration store available.";
   inline constexpr const char* CT_COOKIE_SECRET_NAME           = "CTSession";


}  // namespace ctb::constants