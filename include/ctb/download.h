/*******************************************************************
 * @file download.h
 *
 * @brief Header file functionality in the ctb namespace for
 *        downloading table data from CellarTracker.com
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *
 *******************************************************************/
#pragma once

#include "ctb/CredentialWrapper.h"
#include "ctb/ctb.h"
#include "ctb/tables/table_data.h"
#include "ctb/utility.h"
#include "ctb/utility_http.h"

#include <functional>
#include <string>

namespace ctb
{


   /// @brief the result of a download will contain the requested data if successful, or an Error object if unsuccessful.
   using DownloadResult = std::expected<RawTableData, Error>;

   /// @brief callback functor. Receives updates of the request process, and allows cancellation (return false) for synchronous download
   using ProgressCallback =
      std::function<bool(int64_t downloadTotal, int64_t downloadNow, int64_t uploadTotal, int64_t uploadNow, intptr_t userdata)>;


   /// @brief Retrieve a data table from CT website (synchronous)
   ///
   /// @param cred     - the username/password to use for the download
   /// @param table    - the table to retrieve
   /// @param format   - the data format to return
   /// @param callback - optional callback to receive progress updates
   ///
   /// @param convert_to_utf - If true, the downloaded data is assumed will be converted from Windows-1252 to UTF-8.
   ///                          If false, the downloaded data is returned to the caller un-modified and can be converted as needed (or not)
   ///
   /// @return expected/successful value is the requested table data, unexpected/error value is HTTP status code
   [[nodiscard]] auto downloadRawTableData(const CredentialWrapper& cred,
                                           TableId                  table,
                                           DataFormatId             format,
                                           ProgressCallback*        callback       = nullptr,
                                           bool                     convert_to_utf = true,
                                           uint32_t table_code_page = std::to_underlying(TextEncoding::WINDOWS_1252)) -> DownloadResult;


   /// @brief for async download, the result will be provided through a callback.
   using TableResultCallback = std::move_only_function<void(DownloadResult result)>;


   /// @brief Retrieve a data table from CT website asynchronously
   ///
   /// @param table    - the table to retrieve
   /// @param cred     - the username/password to use for the download
   /// @param callback - callback function to receive the result.
   ///
   void downloadTableAsync(TableId table, const CredentialWrapper& cred, TableResultCallback callback);

}   // namespace ctb
