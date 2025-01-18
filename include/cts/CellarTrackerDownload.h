/*********************************************************************
 * @file       CellarTrackerDownload.h
 *
 * @brief      Declaration for the class CellarTrackerDownload
 *
 * @copyright  Copyright © 2024 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "cts/Error.h"
#include "cts/CredentialWrapper.h"

#include "cpr/callback.h"
#include "frozen/map.h"
#include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum_containers.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace cts::data
{
   /// @brief enum for the data tables available from CT website
   enum class TableId
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
   enum class DataFormatId
   {
      html,	// default if not specified
      xml,
      tab,
      csv
   };


   /// @brief struct that contains data (and metadata) for a downloaded CellarTracker table
   struct TableData
   {
      std::string  data{};
      TableId      table_id{};
      DataFormatId data_format{};

      std::string_view tableName() const noexcept { return magic_enum::enum_name(table_id); }
      std::string_view formatName() const noexcept { return magic_enum::enum_name(data_format); }
   };


   ///
   /// @brief CellarTrackerDownload class retrieves user data from the CellarTracker website.
   /// 
   /// This class downloads table data from the CT website using HTTP requests. 
   ///  
   class CellarTrackerDownload
   {
   public:

      /// @brief the result of a download will contain the requested data if successful, or an Error object if unsuccessful.
      using DownloadResult = std::expected<TableData, Error>;

      /// @brief callback functor. Receives updates of the request process, and allows cancellation (return false)
      using ProgressCallback = std::function<bool(cpr::cpr_pf_arg_t downloadTotal, cpr::cpr_pf_arg_t downloadNow,
                                                  cpr::cpr_pf_arg_t uploadTotal, cpr::cpr_pf_arg_t uploadNow, intptr_t userdata)>;

      /// @brief              retrieve a data table from CT website
      /// @param cred         the username/password to use for the download
      /// @param tbl          the table to retrieve
      /// @param fmt          the data format to return
      /// @param callback_ptr optional callback to receive progress updates
      /// @return             expected/successful value is the requested table data, unexpected/error value is HTTP status code
      [[nodiscard]] static DownloadResult getTableData(const CredentialWrapper::Credential& cred,
                                                     TableId table, DataFormatId format,
                                                     ProgressCallback* callback = nullptr);


      using TableDescriptionMap = frozen::map<TableId, std::string_view, magic_enum::enum_count<TableId>()>;

      /// @brief map enum values for table id's to their long descriptions
      [[nodiscard]] static constexpr TableDescriptionMap const& tableDescriptions() { return m_table_descriptions; }
      [[nodiscard]] static constexpr std::string_view tableDescription(TableId tbl)
      {
         auto it = m_table_descriptions.find(tbl);
         return it == m_table_descriptions.end() ? "" : it->second;
      }

   private:
      static inline constexpr TableDescriptionMap m_table_descriptions{
         { TableId::List, "Personal Wine List" },
         { TableId::Inventory, "Bottle Inventory" },
         { TableId::Notes, "Tasting Notes" },
         { TableId::PrivateNotes, "Private Notes" },
         { TableId::Purchase, "Wine Purchases" },
         { TableId::Pending, "Pending Wine Deliveries" },
         { TableId::Consumed, "Consumed Bottles" },
         { TableId::Availability, "Ready to Drink List" },
         { TableId::Tag, "Wish List Tags" },
         { TableId::ProReview, "Manually Entered Pro Reviews" },
         { TableId::Bottles, "Raw Bottle List" },
         { TableId::FoodTags, "Food Pairing Tags" }
      };


   };


} // namespace cts::data
