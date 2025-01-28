/*******************************************************************
 * @file table_download.h
 *
 * @brief Header file fucnctionality in the ctb::data namespace for
 *        downloading table data from CellarTracker.com
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/data/table_data.h"

namespace ctb::data
{

   /// @brief struct that contains data (and metadata) for a downloaded CellarTracker table
   struct RawTableData
   {
      std::string  data{};
      TableId      table_id{};
      DataFormatId data_format{};

      std::string_view tableName() const noexcept { return magic_enum::enum_name(table_id); }
      std::string_view formatName() const noexcept { return magic_enum::enum_name(data_format); }
   };


   /// @brief the result of a download will contain the requested data if successful, or an Error object if unsuccessful.
   using DownloadResult = std::expected<RawTableData, Error>;


   /// @brief callback functor. Receives updates of the request process, and allows cancellation (return false)
   using ProgressCallback = std::function<bool(__int64 downloadTotal, __int64 downloadNow, __int64 uploadTotal, __int64 uploadNow, intptr_t userdata)>;


   /// @brief              retrieve a data table from CT website
   /// @param cred         the username/password to use for the download
   /// @param tbl          the table to retrieve
   /// @param fmt          the data format to return
   /// @param callback optional callback to receive progress updates
   /// @return             expected/successful value is the requested table data, unexpected/error value is HTTP status code
   [[nodiscard]] DownloadResult downloadRawTableData(const CredentialWrapper::Credential& cred, TableId table, 
                                                     DataFormatId format, ProgressCallback* callback = nullptr);

} // namespace ctb::data