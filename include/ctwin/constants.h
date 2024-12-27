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

   inline constexpr const char* FMT_HTTP_CT_BASE_URL = "https://www.cellartracker.com/xlquery.asp?User={}&Password={}&Format={}&Table={}";
   inline constexpr const char* HTTP_CELLARTRACKER_URL = "https://www.cellartracker.com/xlquery.asp";

   inline constexpr const char* HTTP_HEADER_XCLIENT = "X-Client";
   inline constexpr const char* HTTP_HEADER_XCLIENT_VALUE = "cpr";
   inline constexpr const char* HTTP_PARAM_TABLE = "Table";
   inline constexpr const char* HTTP_PARAM_FORMAT = "Format";

   inline constexpr int MAX_ENV_VAR_LENGTH = 128;
   inline constexpr const char* CT_PASSWORD = "CT_PASSWORD";

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
