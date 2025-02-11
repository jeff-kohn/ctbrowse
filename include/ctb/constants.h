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
   inline constexpr const char* APP_NAME_LONG  = "ctBrowse for Windows";
   inline constexpr const char* APP_NAME_SHORT = "ctBrowse";

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

   inline constexpr const char* TABLE_NAME_LIST             = "Personal Wine List";
   inline constexpr const char* TABLE_NAME_INVENTORY        = "Bottle Inventory";
   inline constexpr const char* TABLE_NAME_NOTES            = "Tasting Notes";
   inline constexpr const char* TABLE_NAME_PRIVATENOTES     = "Private Notes";
   inline constexpr const char* TABLE_NAME_PURCHASE         = "Wine Purchases";
   inline constexpr const char* TABLE_NAME_PENDING          = "Pending Wine Deliveries";
   inline constexpr const char* TABLE_NAME_CONSUMED         = "Consumed Bottles";
   inline constexpr const char* TABLE_NAME_AVAILABILITY     = "Ready to Drink List";
   inline constexpr const char* TABLE_NAME_TAG              = "Wish List Tags";
   inline constexpr const char* TABLE_NAME_PROREVIEW        = "Manually Entered Pro Reviews";
   inline constexpr const char* TABLE_NAME_BOTTLES          = "Raw Bottle List";
   inline constexpr const char* TABLE_NAME_FOODTAGS         = "Food Pairing Tag";

   // CT doesn't return an HTTP response code for invalid logon, it just returns this text instead of the requested file.
   inline constexpr const char* ERR_STR_INVALID_CELLARTRACKER_LOGON = "<html><body>You are currently not logged into CellarTracker.</body></html>";

   inline constexpr const char* ERROR_STR                         = "Error";
   inline constexpr const char* ERROR_STR_AUTHENTICATION_FAILED   = "Invalid username/password.";
   inline constexpr const char* ERROR_STR_DIALOG_TRANSFER_FAILED  = "Unexpected error transferring data to/from dialog.";
   inline constexpr const char* ERROR_STR_DOWNLOAD_AUTH_FAILURE   = "File download canceled, no valid login was provided.";
   inline constexpr const char* ERROR_STR_INVALID_INDEX           = "Invalid index";
   inline constexpr const char* ERROR_STR_INVALID_ROW_INDEX       = "Invalid row index";
   inline constexpr const char* ERROR_STR_EDITING_NOT_SUPPORTED   = "Editing data is not supported.";
   inline constexpr const char* ERROR_STR_NO_GRID_TABLE           = "No data is currently loaded. Open a data view before searching.";
   inline constexpr const char* ERROR_STR_NULLPTR_ARG             = "Invalid Parameter (nullptr).";
   inline constexpr const char* FMT_ERROR_CURL_ERROR              = "The operation failed with CURL error {}";
   inline constexpr const char* FMT_ERROR_FILE_NOT_FOUND          = "File '{}' was not found.";
   inline constexpr const char* FMT_ERROR_HTTP_STATUS_CODE        = "The operation failed with HTTP status code {}";
   inline constexpr const char* FMT_ERROR_PATH_NOT_FOUND          = "Folder '{}' does not exist.";

} // namespace ctb::constants
