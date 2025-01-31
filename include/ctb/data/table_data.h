/*******************************************************************
 * @file table_data.h
 *
 * @brief Header file for functionality in the ctb::data namespace to
 *        work with CT data tables
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"

#pragma warning(push)
#pragma warning(disable: 4365 4464 4702)
#include <external/csv.hpp>
#pragma warning(pop)

#include <frozen/map.h>
#include <magic_enum/magic_enum.hpp>
#include <algorithm>
#include <expected>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace ctb::data
{
   namespace fs = std::filesystem;

   /// @brief some fields with numeric values may not actually have a value
   using NullableDouble = std::optional<double>;
   using NullableShort = std::optional<uint16_t>;


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

   /// @brief default table format (and currently the only format we support parsing)
   inline constexpr auto DEFAULT_TABLE_FORMAT =  DataFormatId::csv;


   /// @brief type alias for a static map of TableId's to display name
   using TableDescriptionMap = frozen::map<TableId, std::string_view, magic_enum::enum_count<TableId>()>;

   /// @brief maps TableId to descriptive name.
   inline constexpr TableDescriptionMap TableDescriptions
   {
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



   /// @brief  returns the descriptive name that corresponds tbl, or empty string if not found
   inline std::string_view getTableDescription(TableId tbl)
   {
      auto it = TableDescriptions.find(tbl);
      if (it != TableDescriptions.end())
         return it->second;
      else
         return "";
   }


   /// @brief  combine enum values to generate a filename.
   inline std::string getTableFileName(TableId tbl, DataFormatId fmt = DEFAULT_TABLE_FORMAT)
   {
      using magic_enum::enum_name;
      return std::format("{}.{}", enum_name(tbl), enum_name(fmt));
   }


   /// @brief  get the fully qualified path for a table 
   inline fs::path getTablePath(fs::path data_folder, TableId tbl, DataFormatId fmt = DEFAULT_TABLE_FORMAT)
   {
      return data_folder / getTableFileName(tbl, fmt);
   }


   /// @brief checks whether the requested table is available at the specified location
   inline bool isTableFileAvailable(fs::path file_path)
   {
      return fs::exists(file_path);
   }


   /// @brief checks whether the requested table is available at the specified location
   inline bool isTableAvailable(fs::path data_folder, TableId tbl, DataFormatId fmt = DEFAULT_TABLE_FORMAT)
   {
      return isTableFileAvailable(getTablePath(data_folder, tbl, fmt));
   }


   /// @brief get a list of available tables in the specified folder
   inline std::vector<TableId> getAvailableTables(fs::path data_folder, DataFormatId fmt = DataFormatId::csv)
   {
      std::vector<TableId> ids{};
      rng::for_each(vws::keys(TableDescriptions), [&](TableId tbl)
         {
            if (isTableAvailable(data_folder, tbl, fmt))
               ids.push_back(tbl);
         });
      return ids;
   }


   /// @brief   load a table object for the given table from disk.
   /// @returns expected value is the requested table object, unexpected value is Error information if operation failed.
   /// 
   /// Note the lack of a "format" parameter, we currently only support parsing CSV files.
   template <typename TableData>
   std::expected<TableData, Error> loadTableData(fs::path data_folder, TableId tbl)
   {
      auto table_path = getTablePath(data_folder, tbl, DataFormatId::csv);
      if (not isTableFileAvailable(table_path))
         return std::unexpected{ Error{ ERROR_FILE_NOT_FOUND, Error::Category::DataError, constants::FMT_ERROR_FILE_NOT_FOUND, table_path.generic_string() } };

      csv::CSVReader reader{ table_path.generic_string() };

      TableData data{};
      for (csv::CSVRow& row : reader)
      {
         typename TableData::value_type record{};
         if (record.parse(row))
            data.emplace_back(std::move(record));
      }
      return data;
   }

} // namespace ctb::data