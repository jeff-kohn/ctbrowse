/// *******************************************************************************
/// @file CellarTrackerDownload.h
/// 
/// @brief this header contains declaration for the CellarTrackerDownload class.
///******************************************************************************** 
#pragma once

#include "cts/Error.h"
#include "cts/CredentialWrapper.h"

#include "magic_enum/magic_enum.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace cts
{
   namespace fs = std::filesystem;

   ///
   /// @brief CellarTrackerDownload class retrieves user data from the CellarTracker website.
   /// 
   /// This class downloads table data from the CT website using HTTP requests. 
   ///  
   class CellarTrackerDownload
   {
   public:
      /// @brief enum for the data tables available from CT website
      enum class Table
      {
         List,			   /// Wine Summary (does not include location or bin unless optional parameter Location=1)
         Inventory,		/// Individual Bottles
         Notes,			/// Tasting Notes
         PrivateNotes,	/// Private Notes
         Purchase,	   /// Purchases
         Pending,	      /// Pending Purchases(Futures)
         Consumed,	   /// Consumed Bottles
         Availability,	/// Ready to Drink(Drinkability) report
         Tag,	         /// Wishlists
         ProReview,	   /// Your manually - entered Professional Reviews
         Bottles,	      /// A special raw view showing all bottles with a BottleState parameter(-1 for pending, 1 for in - stock, 0 for consumed)
         FoodTags	      /// Your food pairing tags
      };


      /// @brief enum for available data formats
      enum class Format
      {
         html,	// default if not specified
         xml,
         tab,
         csv
      };

      struct CellarTrackerTable
      {
         std::string data{};
         Table table{};
         Format data_format{};

         std::string_view tableName() const noexcept  { return magic_enum::enum_name(table);       }
         std::string_view formatName() const noexcept { return magic_enum::enum_name(data_format); }
      };

      /// @brief the result of a download will the requested data if successful, or an Error object if unsuccessful.
      using DownloadResult = std::expected<CellarTrackerTable, Error>;


      /// @brief        retrieve a data table from CT website
      /// @param cred   the username/password to use for the download
      /// @param tbl    the table to retrieve
      /// @param fmt    the data format to return
      /// @return       expected/successful value is the requested table data, unexpected/error value is HTTP status code
      DownloadResult getTableData(const CredentialWrapper::Credential& cred, Table tbl, Format fmt);
   };

} // namespace cts
