/*********************************************************************
 * @file       constants.h
 *
 * @brief      Defines various constants used throughout the project.
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include <cstdint>

namespace ctb::constants
{
   inline constexpr const char* CELLARTRACKER_DOT_COM       = "CellarTracker.com";
   inline constexpr const char* URL_CT_DOT_COM              = "https://cellartracker.com";
   inline constexpr const char* URL_CT_LOGIN_FORM           = "https://www.cellartracker.com/password.asp";
   inline constexpr const char* URL_CT_TABLE_LIST           = "https://www.cellartracker.com/list.asp?Table=List";

   inline constexpr const char* FMT_URL_CT_ACCEPT_PENDING   = "https://www.cellartracker.com/purchase.asp?iWine={}&iPurchase={}&DeliveryState=delivered&DeliveryDate={:%F}";
   inline constexpr const char* FMT_URL_CT_ADD_TASTING_NOTE = "https://www.cellartracker.com/editnote.asp?iWine={}";
   inline constexpr const char* FMT_URL_CT_ADD_TO_CELLAR    = "https://www.cellartracker.com/purchase.asp?iWine={}";
   inline constexpr const char* FMT_URL_CT_DRINK_REMOVE     = "https://www.cellartracker.com/barcode.asp?iWine={}";
   inline constexpr const char* FMT_URL_CT_EDIT_ORDER       = "https://www.cellartracker.com/purchase.asp?iWine={}&iPurchase={}";
   inline constexpr const char* FMT_URL_CT_DRINK_WINDOW     = "https://www.cellartracker.com/editpersonal.asp?iWine={}";
   inline constexpr const char* FMT_URL_CT_TABLE            = "https://www.cellartracker.com/xlquery.asp?User={}&Password={}&Format={}&Table={}";
   inline constexpr const char* FMT_URL_CT_VINTAGES         = "https://www.cellartracker.com/list.asp?Table=List&fInStock=0&iUserOverride=0&Wine={}";
   inline constexpr const char* FMT_URL_CT_WINE_DETAILS     = "https://www.cellartracker.com/wine.asp?iWine={}";

   inline constexpr const char* HTML_ELEM_LABEL_PHOTO       = "label_photo";
   inline constexpr const char* HTML_ATTR_SRC               = "src";

   inline constexpr const char* CELLARTRACKER_LOGON_CAPTION = "Enter login information for CellarTracker.com:";
   inline constexpr const char* CT_PASSWORD                 = "CT_PASSWORD";
   inline constexpr const char* CURRENT_DIRECTORY           = ".";
   inline constexpr const char* DATA_FILE_EXTENSION         = "csv";
   inline constexpr int         MAX_ENV_VAR_LENGTH          = 128;

   // column labels
   inline constexpr const char* DISPLAY_COL_AVAILABLE       = "Ready";
   inline constexpr const char* DISPLAY_COL_LINEAR          = "Linear";
   inline constexpr const char* DISPLAY_COL_BELL_CURVE      = "Bell";
   inline constexpr const char* DISPLAY_COL_EARLY_CURVE     = "Early";
   inline constexpr const char* DISPLAY_COL_LATE_CURVE      = "Late";
   inline constexpr const char* DISPLAY_COL_EARLY_LATE      = "Both";
   inline constexpr const char* DISPLAY_COL_FAST_MATURING   = "Fast";
   inline constexpr const char* DISPLAY_COL_CT_SCORE        = "CT Score";
   inline constexpr const char* DISPLAY_COL_CT_WINDOW       = "CT Window";
   inline constexpr const char* DISPLAY_COL_LOCALE          = "Region and Appellation";
   inline constexpr const char* DISPLAY_COL_MY_SCORE        = "My Score";
   inline constexpr const char* DISPLAY_COL_MY_WINDOW       = "My Window";
   inline constexpr const char* DISPLAY_COL_PURCHASES       = "Purchases";
   inline constexpr const char* DISPLAY_COL_QTY             = "Qty";
   inline constexpr const char* DISPLAY_COL_WINE            = "Wine";
   inline constexpr const char* DISPLAY_COL_PURCH_DATE      = "Date Ordered";
   inline constexpr const char* DISPLAY_COL_STORE           = "Store";
   inline constexpr const char* DISPLAY_COL_PRICE           = "Price";

   // filter names
   inline constexpr const char* FILTER_APPELATION           = "Appellation";
   inline constexpr const char* FILTER_COUNTRY              = "Country";
   inline constexpr const char* FILTER_REGION               = "Region";
   inline constexpr const char* FILTER_STORE                = "Store";
   inline constexpr const char* FILTER_VARIETAL             = "Varietal";
   inline constexpr const char* FILTER_VINTAGE              = "Vintage";
   inline constexpr const char* FILTER_ORDER_DATE           = "Order Date";

   inline constexpr const char* HTTP_PARAM_TABLE            = "Table";
   inline constexpr const char* HTTP_PARAM_FORMAT           = "Format";
   inline constexpr int         HTTP_TIMEOUT_SEC            = 30;
   inline constexpr const char* HTTP_PARAM_KEY_REFERRER     = "Referrer";
   inline constexpr const char* HTTP_PARAM_VAL_REFERRER     = "/default.asp";
   inline constexpr const char* HTTP_PARAM_KEY_USER         = "szUser";
   inline constexpr const char* HTTP_PARAM_KEY_PASSWORD     = "szPassword";
   inline constexpr const char* HTTP_PARAM_KEY_USE_COOKIE   = "UseCookie";
   inline constexpr const char* HTTP_PARAM_VAL_USE_COOKIE   = "true";

   // sort option names
   inline constexpr const char* SORT_OPTION_APPELATION_WINE       = "Appellation, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_COUNTRY_APPELATION    = "Country, Appellation and Wine";
   inline constexpr const char* SORT_OPTION_LOCALE_WINE           = "Country, Region, and Appellation";
   inline constexpr const char* SORT_OPTION_PRODUCER_WINE_NAME    = "Producer, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_REGION_WINE           = "Region, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_PURCHASE_DATE         = "Order Date, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_STORE_NAME            = "Store, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_CURVE_DEFAULT         = "Default Formula";
   inline constexpr const char* SORT_OPTION_CURVE_LINEAR          = "Linear Formula";
   inline constexpr const char* SORT_OPTION_CURVE_BELL            = "Bell Curve Formula";
   inline constexpr const char* SORT_OPTION_CURVE_BELL_EARLY      = "Early Bell Curve Formula";
   inline constexpr const char* SORT_OPTION_CURVE_BELL_LATE       = "Late Bell Curve Formula";
   inline constexpr const char* SORT_OPTION_CURVE_FAST_MATURE     = "Fast Maturing Formula";
   inline constexpr const char* SORT_OPTION_CURVE_EARLY_LATE      = "Early & Late Formula";
   inline constexpr const char* SORT_OPTION_CURVE_BOTTLES_YEAR    = "Bottles per Year Formula";

   inline constexpr const char* SORT_OPTION_SCORE_MY              = "My Score";
   inline constexpr const char* SORT_OPTION_SCORE_CT              = "CellarTracker Score";
   inline constexpr const char* SORT_OPTION_MY_VALUE              = "My Value";
   inline constexpr const char* SORT_OPTION_VARIETAL_WINE         = "Varietal, Wine and Vintage";
   inline constexpr const char* SORT_OPTION_VINTAGE_WINE          = "Vintage and Wine";
   inline constexpr const char* SORT_OPTION_WINE_VINTAGE          = "Wine and Vintage";

   // table names
   inline constexpr const char* TABLE_NAME_LIST                   = "My Wine Cellar";
   inline constexpr const char* TABLE_NAME_INVENTORY              = "Bottle Inventory";
   inline constexpr const char* TABLE_NAME_NOTES                  = "Tasting Notes";
   inline constexpr const char* TABLE_NAME_PRIVATENOTES           = "Private Notes";
   inline constexpr const char* TABLE_NAME_PURCHASE               = "Wine Purchases";
   inline constexpr const char* TABLE_NAME_PENDING                = "Pending Wine Deliveries";
   inline constexpr const char* TABLE_NAME_CONSUMED               = "Consumed Bottles";
   inline constexpr const char* TABLE_NAME_AVAILABILITY           = "Ready to Drink List";
   inline constexpr const char* TABLE_NAME_TAG                    = "Wish List Tags";
   inline constexpr const char* TABLE_NAME_PROREVIEW              = "Manually Entered Pro Reviews";
   inline constexpr const char* TABLE_NAME_BOTTLES                = "Raw Bottle List";
   inline constexpr const char* TABLE_NAME_FOODTAGS               = "Food Pairing Tag";

   // CT doesn't return an HTTP response code for invalid logon, it just returns this text instead of the requested file.
   inline constexpr const char* ERR_STR_INVALID_CELLARTRACKER_LOGON = "<html><body>You are currently not logged into CellarTracker.</body></html>";

   inline constexpr const char* ERROR_STR                         = "Error";
   inline constexpr const char* ERROR_STR_AUTHENTICATION_FAILED   = "Invalid username/password.";
   inline constexpr const char* ERROR_STR_LABEL_URL_NOT_FOUND     = "Label Image URL not found in HTML.";
   inline constexpr const char* ERROR_STR_OPERATION_CANCELED      = "Operation Canceled.";
   inline constexpr const char* ERROR_STR_DIALOG_TRANSFER_FAILED  = "Unexpected error transferring data to/from dialog.";
   inline constexpr const char* ERROR_STR_DOWNLOAD_AUTH_FAILURE   = "File download canceled, no valid login was provided.";
   inline constexpr const char* ERROR_STR_INVALID_INDEX           = "Invalid index";
   inline constexpr const char* ERROR_STR_INVALID_ROW_INDEX       = "Invalid row index";
   inline constexpr const char* ERROR_STR_EDITING_NOT_SUPPORTED   = "Editing data is not supported.";
   inline constexpr const char* ERROR_STR_NO_GRID_TABLE           = "No data is currently loaded. Open a data view before searching.";
   inline constexpr const char* ERROR_STR_NULLPTR_ARG             = "Invalid Parameter (nullptr).";
   inline constexpr const char* ERROR_STR_RELATIVE_LABEL_CACHE    = "The label image cache folder must be an absolute path, not relative.";
   inline constexpr const char* ERORR_STR_NO_SECRET_STORE         = "Secret Store not available.";
   inline constexpr const char* ERROR_STR_UNKNOWN                 = "Unknown Error.";
   inline constexpr const char* ERROR_VAL                         = "#Err#";
   inline constexpr const char* FMT_ERROR_CURL_ERROR              = "The operation failed with CURL error {}";
   inline constexpr const char* FMT_ERROR_FILE_NOT_FOUND          = "File '{}' was not found.";
   inline constexpr const char* FMT_ERROR_FILE_ALREADY_EXISTS     = "File '{}' already exists and will not be overwritten.";
   inline constexpr const char* FMT_ERROR_FILE_OPEN_FAILED        = "File '{}' could not be opened.";
   inline constexpr const char* FMT_ERROR_FILE_READ_FAILED        = "Read operation on file '{}' failed.";
   inline constexpr const char* FMT_ERROR_FILE_WRITE_FAILED       = "Write operation on file '{}' failed.";
   inline constexpr const char* FMT_ERROR_FILE_TOO_BIG            = "File '{}'s size of {} bytes exceeded the maximum allowable size of {}.";
   inline constexpr const char* FMT_ERROR_HTTP_STATUS_CODE        = "The operation failed with HTTP status code {}";
   inline constexpr const char* FMT_ERORR_NO_CREDENTIAL           = "Credential '{}' was not found or could not be loaded."; 
   inline constexpr const char* FMT_ERROR_NO_LABEL_CACHE_FOLDER   = "The image cache folder {} does not exist and could not be created.";
   inline constexpr const char* FMT_ERROR_PATH_NOT_FOUND          = "Folder '{}' does not exist.";
   inline constexpr const char* FMT_ERROR_PROP_NOT_FOUND          = "Property '{}' was not found.";

   inline constexpr const char* FMT_NUMBER_CURRENCY               = "${:.2f}";
   inline constexpr const char* FMT_NUMBER_DECIMAL                = "{:.1f}";
   inline constexpr const char* FMT_DATE_SHORT                    = "{:%m/%d/%Y}";
   inline constexpr const char* FMT_ISO_SHORT_DATE_ONLY           = "{:%F}";
   inline constexpr const char* FMT_NUMBER_INTEGRAL               = "{:.0f}";
   inline constexpr const char* FMT_PARSE_DATE_SHORT              = "%m/%d/%Y";
   inline constexpr const char* FMT_PARSE_ISO_DATETIME_LOCAL      = "%FT%T%Ez"; //. These 3 don't have {}'s because they're meant to be used with std::chrono::parse
   inline constexpr const char* FMT_PARSE_ISO_DATETIME_UTC        = "%FT%T%Z";
   inline constexpr const char* FMT_PARSE_ISO_DATE_ONLY           = "%F";

   inline constexpr auto     ONE_MB                               = 1024 * 1024;
   inline constexpr uint16_t CT_NULL_YEAR                         =        9999;
   inline constexpr double   FILTER_SCORE_DEFAULT                 =        90.0;
   inline constexpr double   FILTER_SCORE_INCR                    =         0.1;
   inline constexpr double   FILTER_SCORE_MIN                     =          50;
   inline constexpr double   FILTER_SCORE_MAX                     =       100.0;
   inline constexpr int      FILTER_SCORE_DIGITS                  =           1;
   inline constexpr double   FILTER_AVAILABLE_MIN_QTY             =       0.999;


} // namespace ctb::constants
