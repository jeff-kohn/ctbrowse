#pragma once

#include "cts/concepts.h"

#include <frozen/map.h>
#include <magic_enum/magic_enum.hpp>
#include <expected>
#include <filesystem>
#include <optional>
#include <string_view>

namespace cts::data
{
   namespace fs = std::filesystem;

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


   /// @brief nullable/optional string_view
   using MaybeStringView = std::optional<std::string_view>;

   /// @brief  returns the 
   inline MaybeStringView getTableDescription(TableId tbl_id)
   {
      auto it = TableDescriptions.find(tbl_id);
      if (it != TableDescriptions.end())
         return MaybeStringView{ it->second };
      else
         return std::nullopt;
   }


   /// @brief struct that contains data (and metadata) for a downloaded CellarTracker table
   struct RawTableData
   {
      std::string  data{};
      TableId      table_id{};
      DataFormatId data_format{};

      std::string_view tableName() const noexcept { return magic_enum::enum_name(table_id); }
      std::string_view formatName() const noexcept { return magic_enum::enum_name(data_format); }
   };

   /// @brief get a list of available tables in the specified folder
   inline std::vector<TableId> getAvailableTables(fs::path data_folder, DataFormatId fmt)
   {
      using magic_enum::enum_name;

      std::vector<TableId> ids{};
      rng::for_each(vws::keys(TableDescriptions), [&](TableId tbl)
         {
            std::string filename = std::format("{}.{}", enum_name(tbl), enum_name(fmt));
         });
   }


} // namespace cts::data