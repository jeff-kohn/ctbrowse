/*********************************************************************
 * @file       constants.h
 *
 * @brief      Defines various constants used throughout the project.
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once


namespace cts::constants
{
   inline constexpr const char* APP_NAME_LONG  = "CellarTracker Search for Windows";
   inline constexpr const char* APP_NAME_SHORT = "cts_win";

   inline constexpr const char* CELLARTRACKER_DOT_COM = "CellarTracker.com";
   inline constexpr const char* CELLARTRACKER_LOGON_CAPTION = "Enter login information for CellarTracker.com:";
   inline constexpr const char* CELLARTRACKER_LOGON_TITLE = "CellarTracker Login";

   inline constexpr const char* FMT_HTTP_CELLARTRACKER_QUERY_URL = "https://www.cellartracker.com/xlquery.asp?User={}&Password={}&Format={}&Table={}";

   inline constexpr const char* HTTP_HEADER_XCLIENT = "X-Client";
   inline constexpr const char* HTTP_HEADER_XCLIENT_VALUE = APP_NAME_SHORT;
   inline constexpr const char* HTTP_PARAM_TABLE = "Table";
   inline constexpr const char* HTTP_PARAM_FORMAT = "Format";

   inline constexpr int MAX_ENV_VAR_LENGTH = 128;
   inline constexpr const char* CT_PASSWORD = "CT_PASSWORD";

   // CT doesn't return an HTTP response code for invalid logon, it just returns this text instead of the requested file.
   inline constexpr const char* ERR_INVALID_CELLARTRACKER_LOGON = "<html><body>You are currently not logged into CellarTracker.</body></html>";

   inline constexpr const char* ERROR_AUTHENTICATION_FAILED = "Invalid username/password.";
   inline constexpr const char* ERROR_FMT_CURL_ERROR = "The operation failed with CURL error {}";
   inline constexpr const char* ERROR_FMT_HTTP_STATUS_CODE = "The operation failed with HTTP status code {}";

} // namespace cts::constants
