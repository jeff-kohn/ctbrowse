/*******************************************************************
 * @file table_data.h
 *
 * @brief Header file for functionality in the ctb namespace to
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

#include <string>
#include <string_view>
#include <vector>


namespace ctb
{
   namespace fs = std::filesystem;


   /// @brief enum for the data tables available from CT website. The enum names are important because
   ///        they map to the filenames used by CellarTracker.
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
      { TableId::List,           constants::TABLE_NAME_LIST          },
      { TableId::Inventory,      constants::TABLE_NAME_INVENTORY     },
      { TableId::Notes,          constants::TABLE_NAME_NOTES         },
      { TableId::PrivateNotes,   constants::TABLE_NAME_PRIVATENOTES  },
      { TableId::Purchase,       constants::TABLE_NAME_PURCHASE      },
      { TableId::Pending,        constants::TABLE_NAME_PENDING       },
      { TableId::Consumed,       constants::TABLE_NAME_CONSUMED      },
      { TableId::Availability,   constants::TABLE_NAME_AVAILABILITY  },
      { TableId::Tag,            constants::TABLE_NAME_TAG           },
      { TableId::ProReview,      constants::TABLE_NAME_PROREVIEW     },
      { TableId::Bottles,        constants::TABLE_NAME_BOTTLES       },
      { TableId::FoodTags,       constants::TABLE_NAME_FOODTAGS      }
   };



   /// @brief  Returns the user-facing descriptive name for a table, or empty string if not found
   ///
   inline auto getTableDescription(TableId tbl) -> std::string_view
   {
      auto it = TableDescriptions.find(tbl);
      if (it != TableDescriptions.end())
         return it->second;
      else
         return "";
   }


   /// @brief  combine enum values to generate a filename.
   ///
   inline auto getTableFileName(TableId tbl, DataFormatId fmt = DEFAULT_TABLE_FORMAT) -> std::string
   {
      using magic_enum::enum_name;
      return ctb::format("{}.{}", enum_name(tbl), enum_name(fmt));
   }


   /// @brief  get the fully qualified path for a table 
   ///
   inline auto getTablePath(fs::path data_folder, TableId tbl, DataFormatId fmt = DEFAULT_TABLE_FORMAT) -> fs::path
   {
      return data_folder / getTableFileName(tbl, fmt);
   }


   /// @brief checks whether the requested table is available at the specified location
   ///
   inline auto isTableFileAvailable(fs::path file_path) -> bool
   {
      return fs::exists(file_path);
   }


   /// @brief checks whether the requested table is available at the specified location
   ///
   inline auto isTableAvailable(fs::path data_folder, TableId tbl, DataFormatId fmt = DEFAULT_TABLE_FORMAT) -> bool
   {
      return isTableFileAvailable(getTablePath(data_folder, tbl, fmt));
   }


   /// @brief get a list of available tables in the specified folder
   ///
   inline auto getAvailableTables(fs::path data_folder, DataFormatId fmt = DataFormatId::csv) -> std::vector<TableId>
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
   ///
   template <typename TableDataT>
   auto loadTableData(fs::path data_folder, TableId tbl) -> std::expected<TableDataT, Error>
   {
      auto table_path = getTablePath(data_folder, tbl, DataFormatId::csv);
      if (not isTableFileAvailable(table_path))
         return std::unexpected{ Error{ ERROR_FILE_NOT_FOUND, Error::Category::DataError, constants::FMT_ERROR_FILE_NOT_FOUND, table_path.generic_string() } };

      csv::CSVReader reader{ table_path.generic_string() };

      TableDataT data{};
      for (csv::CSVRow& row : reader)
      {
         data.emplace_back(row);
      }
      return data;
   }


} // namespace ctb