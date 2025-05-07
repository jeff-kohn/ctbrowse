/**************************************************************************************************
* @file  PendingWineTable.h
*
* @brief defines the PendingWineTable class, which is an instantiation
*        of CtDataTable<> implemented using the traits template PendingWineTraits 
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
**************************************************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/CtDataTable.h"
#include <frozen/map.h>

#include <utility>
#include <span>
#include <vector>

namespace ctb
{
   using detail::FieldSchema;
   using detail::PropType;
   using detail::TableProperty;
   using detail::TableRecord;

   /// @brief Traits class for a table record from the 'Pending Wine' CellarTracker CSV table.
   /// 
   class PendingWineTraits
   {
   public:
      using PropId = CtPropId;

      /// @brief - contains the list of data fields that are parsed from CSV
      /// 
      /// this collection contains the list of properties that we actually parse from the CSV file. Any
      /// calculated properties are not included here which explains why this array doesn't contain 
      /// every PropId enum value.
      /// 
      static inline constexpr frozen::map<PropId, FieldSchema, static_cast<size_t>(PropId::WineAndVintage)> CsvSchema
      {
         { PropId::iWineId,                FieldSchema { static_cast<uint32_t>(PropId::iWineId),               PropType::String,      0 }},
         { PropId::PendingPurchaseId,      FieldSchema { static_cast<uint32_t>(PropId::PendingPurchaseId),     PropType::String,      1 }},
         { PropId::WineName,               FieldSchema { static_cast<uint32_t>(PropId::WineName),              PropType::String,     17 }},
         { PropId::Locale,                 FieldSchema { static_cast<uint32_t>(PropId::Locale),                PropType::String,     19 }},
         { PropId::Vintage,                FieldSchema { static_cast<uint32_t>(PropId::Vintage),               PropType::UInt16,     17 }},
         { PropId::PendingPurchaseDate,    FieldSchema { static_cast<uint32_t>(PropId::PendingPurchaseDate),   PropType::Date,        2 }}, 
         { PropId::PendingDeliveryDate,    FieldSchema { static_cast<uint32_t>(PropId::PendingDeliveryDate),   PropType::Date,        3 }}, 
         { PropId::PendingStoreName,       FieldSchema { static_cast<uint32_t>(PropId::PendingStoreName),      PropType::String,      4 }},
         { PropId::QtyPending,             FieldSchema { static_cast<uint32_t>(PropId::QtyPending),            PropType::UInt16,     10 }},
         { PropId::PendingQtyOrdered,      FieldSchema { static_cast<uint32_t>(PropId::PendingQtyOrdered),     PropType::UInt16,     11 }},
         { PropId::PendingOrderNumber,     FieldSchema { static_cast<uint32_t>(PropId::PendingOrderNumber),    PropType::String,     12 }},
         { PropId::Size,                   FieldSchema { static_cast<uint32_t>(PropId::Size),                  PropType::String,     14 }},
         { PropId::PendingPrice,           FieldSchema { static_cast<uint32_t>(PropId::PendingPrice),          PropType::Double,      8 }},
         { PropId::Currency,               FieldSchema { static_cast<uint32_t>(PropId::Currency),              PropType::String,      9 }},
         { PropId::Country,                FieldSchema { static_cast<uint32_t>(PropId::Country),               PropType::String,     28 }},
         { PropId::Region,                 FieldSchema { static_cast<uint32_t>(PropId::Region),                PropType::String,     29 }},
         { PropId::SubRegion,              FieldSchema { static_cast<uint32_t>(PropId::SubRegion),             PropType::String,     30 }},
         { PropId::Appellation,            FieldSchema { static_cast<uint32_t>(PropId::Appellation),           PropType::String,     31 }},
         { PropId::Producer,               FieldSchema { static_cast<uint32_t>(PropId::Producer),              PropType::String,     23 }},
         { PropId::Color,                  FieldSchema { static_cast<uint32_t>(PropId::Color),                 PropType::String,     21 }},
         { PropId::Category,               FieldSchema { static_cast<uint32_t>(PropId::Category),              PropType::String,     22 }},
         { PropId::Varietal,               FieldSchema { static_cast<uint32_t>(PropId::Varietal),              PropType::String,     25 }}
      };

      /// @brief getCsvSchema()
      /// @return the CSV schema for this CT table
      /// 
      static constexpr auto getCsvSchema() -> const auto&
      {
         return CsvSchema;
      }

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      /// 
      static constexpr auto getTableName() -> std::string_view 
      { 
         return "PendingWine"; 
      }

      /// @brief small helper to convert a Prop enum into its integer index
      /// 
      static constexpr auto propToIndex(PropId prop) -> int 
      {
         return enumToIndex(prop);
      }

      /// @brief small helper to convert a zero-based index to a Prop enum
      /// 
      static constexpr auto propFromIndex(int idx) -> PropId
      {
         return enumFromIndex<PropId>(idx);
      }

      static constexpr auto supportsInStockFilter() -> bool
      {
         return false;
      }

      /// @brief this gets called by TableRecord to set any missing property values
      /// 
      /// Properties from the CSV file are already set, this impl just provides
      /// any calculated property values or does fixup for any parsed values that need it.
      /// 
      /// @param rec span containing a TableProperty for each PropID enum value.
      /// 
      template<std::size_t N, typename... Args>
      static void onRecordParse(std::span<TableProperty<Args...>, N> rec)
      {
         using enum PropId;

         // set value for the WineAndVintage property
         auto vintage   = rec[static_cast<size_t>(PropId::Vintage) ].asString();
         auto wine_name = rec[static_cast<size_t>(PropId::WineName)].asStringView();
         rec[static_cast<size_t>(PropId::WineAndVintage)] = ctb::format("{} {}", vintage, wine_name);
      }
   };


   /// @brief Type alias for a CtDataTable representing the PendingWines table.
   ///
   using PendingWineTable  = CtDataTable<PendingWineTraits>;

} // namespace ctb