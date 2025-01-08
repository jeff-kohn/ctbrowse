//---------------------------------------------------------------------------------------------------------------------
// constants.h
//
// Defines various constants used throughout the project.
//
// Copyright (c) 2024 Jeff Kohn. All Right Reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace ctwin::constants
{
   inline constexpr const char* APP_NAME = "CT-Win";

   inline constexpr const char* CELLARTRACKER_DOT_COM = "CellarTracker.com";
   inline constexpr const char* CELLARTRACKER_LOGON_CAPTION = "Enter login information for CellarTracker.com:";
   inline constexpr const char* CELLARTRACKER_LOGON_TITLE = "CellarTracker Login";

   inline constexpr const char* FMT_HTTP_CELLARTRACKER_QUERY_URL = "https://www.cellartracker.com/xlquery.asp?User={}&Password={}&Format={}&Table={}";

   inline constexpr const char* HTTP_HEADER_XCLIENT = "X-Client";
   inline constexpr const char* HTTP_HEADER_XCLIENT_VALUE = "cpr";
   inline constexpr const char* HTTP_PARAM_TABLE = "Table";
   inline constexpr const char* HTTP_PARAM_FORMAT = "Format";

   inline constexpr int MAX_ENV_VAR_LENGTH = 128;
   inline constexpr const char* CT_PASSWORD = "CT_PASSWORD";

   // CT doesn't return an HTTP response code for invalid logon, it just returns this text instead of the request file.
   inline constexpr const char* ERR_INVALID_CELLARTRACKER_LOGON = "<html><body>You are currently not logged into CellarTracker.</body></html>";

   inline constexpr const char* ERROR_AUTHENTICATION_FAILED = "Invalid username/password.";
   inline constexpr const char* ERROR_FMT_CURL_ERROR = "The operation failed with CURL error {}";
   inline constexpr const char* ERROR_FMT_HTTP_STATUS_CODE = "The operation failed with HTTP status code {}";

   //inline constexpr const char* ERROR_CATEGORY_SUCCESS = "Success";
   //inline constexpr const char* ERROR_CATEGORY_GENERIC = "Generic Runtime Error";
   //inline constexpr const char* ERROR_CATEGORY_REST = "REST Error";
   //inline constexpr const char* ERROR_CATEGORY_PARSE = "Parsing Error";
   //inline constexpr const char* ERROR_CATEGORY_FILE_IO = "File I/O Error";
   //inline constexpr const char* ERROR_CATEGORY_QUERY = "Query error";
   //inline constexpr const char* ERROR_CATEGORY_SCHEMA = "Schema error";
   //inline constexpr const char* ERROR_CONTEXT_REST_RESPONSE = "REST Response";


   inline constexpr const char* CONFIG_DEFAULT_LOG_FOLDER = "%LOCALAPPDATA%/oura_charts/logs";


} // namespace ctwin::constants
