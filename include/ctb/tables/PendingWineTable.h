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

#include <span>


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
      using CtProp      = CtProp;
      using FieldSchema = FieldSchema<CtProp>;
      using SchemaMap   = frozen::map<CtProp, FieldSchema, static_cast<size_t>(CtProp::WineAndVintage)>;


      static inline constexpr std::array s_supported_properties = 
         {
            CtProp::iWineId,
            CtProp::WineName,
            CtProp::Locale,
            CtProp::Vintage,
            CtProp::Producer,
            CtProp::Country,
            CtProp::Region,
            CtProp::SubRegion,
            CtProp::Appellation,
            CtProp::Color,
            CtProp::Category,
            CtProp::Varietal,
            CtProp::QtyPending,
            CtProp::Size,
            CtProp::Currency,
            CtProp::PendingPrice,
            CtProp::PendingPurchaseId,
            CtProp::PendingStoreName,
            CtProp::PendingOrderNumber,
            CtProp::PendingQtyOrdered,
            CtProp::PendingPurchaseDate,
            CtProp::PendingDeliveryDate,
            CtProp::WineAndVintage,
            CtProp::QtyTotal
         };


      /// @brief - contains the list of data fields that are parsed from CSV
      /// 
      /// this collection contains the list of properties that we actually parse from the CSV file. Any
      /// calculated properties are not included here which explains why this array doesn't contain 
      /// every CtProp enum value.
      /// 
      static inline constexpr SchemaMap s_csv_schema
      {
         { CtProp::iWineId,                FieldSchema { CtProp::iWineId,               PropType::String,      0 }},
         { CtProp::WineName,               FieldSchema { CtProp::WineName,              PropType::String,     17 }},
         { CtProp::Locale,                 FieldSchema { CtProp::Locale,                PropType::String,     19 }},
         { CtProp::Vintage,                FieldSchema { CtProp::Vintage,               PropType::UInt16,     17 }},
         { CtProp::Producer,               FieldSchema { CtProp::Producer,              PropType::String,     23 }},
         { CtProp::Country,                FieldSchema { CtProp::Country,               PropType::String,     28 }},
         { CtProp::Region,                 FieldSchema { CtProp::Region,                PropType::String,     29 }},
         { CtProp::SubRegion,              FieldSchema { CtProp::SubRegion,             PropType::String,     30 }},
         { CtProp::Appellation,            FieldSchema { CtProp::Appellation,           PropType::String,     31 }},
         { CtProp::Color,                  FieldSchema { CtProp::Color,                 PropType::String,     21 }},
         { CtProp::Category,               FieldSchema { CtProp::Category,              PropType::String,     22 }},
         { CtProp::Varietal,               FieldSchema { CtProp::Varietal,              PropType::String,     25 }},
         { CtProp::QtyPending,             FieldSchema { CtProp::QtyPending,            PropType::UInt16,     10 }},
         { CtProp::Size,                   FieldSchema { CtProp::Size,                  PropType::String,     14 }},
         { CtProp::Currency,               FieldSchema { CtProp::Currency,              PropType::String,      9 }},
         { CtProp::PendingPrice,           FieldSchema { CtProp::PendingPrice,          PropType::Double,      8 }},
         { CtProp::PendingPurchaseId,      FieldSchema { CtProp::PendingPurchaseId,     PropType::String,      1 }},
         { CtProp::PendingStoreName,       FieldSchema { CtProp::PendingStoreName,      PropType::String,      4 }},
         { CtProp::PendingOrderNumber,     FieldSchema { CtProp::PendingOrderNumber,    PropType::String,     12 }},
         { CtProp::PendingQtyOrdered,      FieldSchema { CtProp::PendingQtyOrdered,     PropType::UInt16,     11 }},
         { CtProp::PendingPurchaseDate,    FieldSchema { CtProp::PendingPurchaseDate,   PropType::Date,        2 }}, 
         { CtProp::PendingDeliveryDate,    FieldSchema { CtProp::PendingDeliveryDate,   PropType::Date,        3 }} 
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
      static constexpr auto propToIndex(CtProp prop) -> int 
      {
         return enumToIndex(prop);
      }

      /// @brief small helper to convert a zero-based index to a Prop enum
      /// 
      static constexpr auto propFromIndex(int idx) -> CtProp
      {
         return enumFromIndex<CtProp>(idx);
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
         using enum CtProp;

         // set value for the WineAndVintage property
         auto vintage   = rec[static_cast<size_t>(CtProp::Vintage) ].asString();
         auto wine_name = rec[static_cast<size_t>(CtProp::WineName)].asStringView();
         rec[static_cast<size_t>(CtProp::WineAndVintage)] = ctb::format("{} {}", vintage, wine_name);
      }
   };


   /// @brief Type alias for a CtDataTable representing the PendingWines table.
   ///
   using PendingWineTable  = CtDataTable<PendingWineTraits>;

} // namespace ctb