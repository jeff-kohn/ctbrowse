/*********************************************************************
 * @file       constants.h
 *
 * @brief      Defines various constants used throughout the project.
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include <string_view>
#include <format>

namespace ctb::constants
{
   inline constexpr const char* APP_NAME_LONG  = "CellarTracker Browse for Windows";
   inline constexpr const char* APP_NAME_SHORT = "cts_win";

   inline constexpr const char* FMT_HTTP_CELLARTRACKER_QUERY_URL = "https://www.cellartracker.com/xlquery.asp?User={}&Password={}&Format={}&Table={}";

   inline constexpr const char* CELLARTRACKER_DOT_COM       = "CellarTracker.com";
   inline constexpr const char* CELLARTRACKER_LOGON_CAPTION = "Enter login information for CellarTracker.com:";
   inline constexpr const char* CELLARTRACKER_LOGON_TITLE   = "CellarTracker Login";

   inline constexpr const char* HTTP_HEADER_XCLIENT         = "X-Client";
   inline constexpr const char* HTTP_HEADER_XCLIENT_VALUE   = APP_NAME_SHORT;
   inline constexpr const char* HTTP_PARAM_TABLE            = "Table";
   inline constexpr const char* HTTP_PARAM_FORMAT           = "Format";
   inline constexpr int         HTTP_TIMEOUT_SEC            = 30;

   inline constexpr const char* DATA_FILE_EXTENSION         = "csv";
   inline constexpr int         MAX_ENV_VAR_LENGTH          = 128;
   inline constexpr const char* CT_PASSWORD                 = "CT_PASSWORD";
   inline constexpr const char* CURRENT_DIRECTORY           = ".";

   // CT doesn't return an HTTP response code for invalid logon, it just returns this text instead of the requested file.
   inline constexpr const char* ERR_INVALID_CELLARTRACKER_LOGON = "<html><body>You are currently not logged into CellarTracker.</body></html>";

   inline constexpr const char* ERROR_STR                      = "Error";
   inline constexpr const char* ERROR_AUTHENTICATION_FAILED    = "Invalid username/password.";
   inline constexpr const char* ERROR_DIALOG_TRANSFER_FAILED   = "Unexpected error transferring data to/from dialog.";
   inline constexpr const char* ERROR_DOWNLOAD_AUTH_FAILURE    = "File download canceled, no valid login was provided.";
   inline constexpr const char* ERROR_INVALID_PROP_INDEX       = "Invalid property index";
   inline constexpr const char* ERROR_INVALID_ROW_INDEX        = "Invalid row index";
   inline constexpr const char* ERROR_EDITING_NOT_SUPPORTED    = "Editing data is not supported.";
   inline constexpr const char* FMT_ERROR_FILE_NOT_FOUND       = "File '{}' was not found.";
   inline constexpr const char* FMT_ERROR_PATH_NOT_FOUND       = "Folder '{}' does not exist.";
   inline constexpr const char* FMT_ERROR_CURL_ERROR           = "The operation failed with CURL error {}";
   inline constexpr const char* FMT_ERROR_HTTP_STATUS_CODE     = "The operation failed with HTTP status code {}";

   inline constexpr const char* CONFIG_PATH_SYNC                  = "/Preferences/DataSync";
   inline constexpr const char* CONFIG_VALUE_SYNC_ON_STARTUP      = "SyncOnStartup";
   inline constexpr const char* CONFIG_VALUE_DEFAULT_SYNC_TABLES  = "DefaultSyncTables";

   inline constexpr const char* TABLE_DESCRIPTION_LIST            = "Personal Wine List";
   inline constexpr const char* TABLE_DESCRIPTION_INVENTORY       = "Bottle Inventory";
   inline constexpr const char* TABLE_DESCRIPTION_NOTES           = "Tasting Notes";
   inline constexpr const char* TABLE_DESCRIPTION_PRIVATE_NOTES   = "Private Notes";
   inline constexpr const char* TABLE_DESCRIPTION_PURCHASE        = "Wine Purchases";
   inline constexpr const char* TABLE_DESCRIPTION_PENDING         = "Pending Wine Deliveries";
   inline constexpr const char* TABLE_DESCRIPTION_CONSUMED        = "Consumed Bottles";
   inline constexpr const char* TABLE_DESCRIPTION_AVAILABILITY    = "Ready to Drink List";
   inline constexpr const char* TABLE_DESCRIPTION_TAG             = "Tags";
   inline constexpr const char* TABLE_DESCRIPTION_PRO_REVIEW      = "Manually Entered Pro Reviews";
   inline constexpr const char* TABLE_DESCRIPTION_BOTTLES         = "Raw Bottle List";
   inline constexpr const char* TABLE_DESCRIPTION_FOOD_TAGS       = "Food Pairing Tags";

   inline constexpr const char* FMT_STATUS_FILE_DOWNLOADING       = "Downloading file '{}'...";
   inline constexpr const char* FMT_STATUS_FILE_DOWNLOADED        = "Successfully downloaded file '{}'.";
   inline constexpr const char* STATUS_DOWNLOAD_COMPLETE          = "Download operation completed.";
   inline constexpr const char* STATUS_DOWNLOAD_CANCELED          = "Download operation canceled.";
   inline constexpr const char* STATUS_DOWNLOAD_FAILED            = "Download operation failed.";

   inline constexpr const char* LBL_WINE                          = "Wine";
   inline constexpr const char* LBL_VARIETAL                      = "Varietal";
   inline constexpr const char* LBL_QTY                           = "Qty";
   inline constexpr const char* LBL_CT_SCORE                      = "CT Score";
   inline constexpr const char* LBL_MY_SCORE                      = "My Score";
   inline constexpr const char* LBL_MY_PRICE                      = "My Price";
   inline constexpr const char* LBL_DRINK_BY                      = "Drink By";
   inline constexpr const char* LBL_SEARCH                        = "&Search";

   inline constexpr const char* TITLE_DOWNLOAD_DATA               = "Download Data";




} // namespace ctb::constants
