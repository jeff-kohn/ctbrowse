/**************************************************************************************************
* @file  WineListTable.h
*
* @brief defines the WineListTable class, which is an instantiation of CtDataTable<> 
*        implemented using the traits template WineListTraits 
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
**************************************************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/table_data.h"

#include "ctb/tables/CtDataTable.h"
#include <boost/unordered/unordered_flat_map.hpp>

#include <frozen/map.h>
#include <span>


namespace ctb
{
   /// @brief Traits class for a table record from the 'List' CellarTracker CSV table.
   /// 
   class WineListTraits
   {
   public:
      using Prop        = CtProp;
      using Property    = CtProperty;
      using PropertyMap = CtPropertyMap;
      using FieldSchema = FieldSchema;

   private:
      /// @brief - contains the list of data fields that are parsed from CSV
      /// 
      /// this collection contains the list of properties that we actually parse from the CSV file. Any
      /// calculated properties are not included here which explains why this array doesn't contain 
      /// every Prop enum value.
      /// 
      static inline constexpr auto s_schema = frozen::make_map<Prop, FieldSchema>(
      {
         { Prop::iWineId,         FieldSchema { Prop::iWineId,        PropType::String,      0 }},
         { Prop::WineName,        FieldSchema { Prop::WineName,       PropType::String,     13 }},
         { Prop::Locale,          FieldSchema { Prop::Locale,         PropType::String,     14 }},
         { Prop::Vintage,         FieldSchema { Prop::Vintage,        PropType::UInt16,     12 }},
         { Prop::Producer,        FieldSchema { Prop::Producer,       PropType::String,     19 }},
         { Prop::Country,         FieldSchema { Prop::Country,        PropType::String,     15 }},
         { Prop::Region,          FieldSchema { Prop::Region,         PropType::String,     16 }},
         { Prop::SubRegion,       FieldSchema { Prop::SubRegion,      PropType::String,     17 }},
         { Prop::Appellation,     FieldSchema { Prop::Appellation,    PropType::String,     18 }},
         { Prop::Color,           FieldSchema { Prop::Color,          PropType::String,     22 }},
         { Prop::Category,        FieldSchema { Prop::Category,       PropType::String,     23 }},
         { Prop::Varietal,        FieldSchema { Prop::Varietal,       PropType::String,     25 }},
         { Prop::CtScore,         FieldSchema { Prop::CtScore,        PropType::Double,     59 }},
         { Prop::MyScore,         FieldSchema { Prop::MyScore,        PropType::Double,     61 }},
         { Prop::QtyOnHand,       FieldSchema { Prop::QtyOnHand,      PropType::UInt16,      2 }},
         { Prop::QtyPending,      FieldSchema { Prop::QtyPending,     PropType::UInt16,      3 }},
         { Prop::Size,            FieldSchema { Prop::Size,           PropType::String,      4 }},
         { Prop::BeginConsume,    FieldSchema { Prop::BeginConsume,   PropType::UInt16,     63 }},
         { Prop::EndConsume,      FieldSchema { Prop::EndConsume,     PropType::UInt16,     64 }},
         { Prop::MyPrice,         FieldSchema { Prop::MyPrice,        PropType::Double,      5 }},
         { Prop::CtPrice,         FieldSchema { Prop::CtPrice,        PropType::Double,      9 }},
         { Prop::AuctionPrice,    FieldSchema { Prop::AuctionPrice,   PropType::Double,      8 }},
         { Prop::WineAndVintage,  FieldSchema { Prop::WineAndVintage, PropType::Double,     {} }},
         { Prop::QtyTotal,        FieldSchema { Prop::QtyTotal,       PropType::Double,     {} }}
      });

   public:
      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      /// 
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::List; 
      }

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      /// 
      static constexpr auto getTableName() -> std::string_view 
      { 
         return "WineList"; 
      }

      using SchemaMap = decltype(s_schema);

      /// @brief getCsvSchema()
      /// @return the CSV schema for this CT table
      /// 
      static constexpr auto getSchema() -> const SchemaMap&
      {
         return s_schema;
      }

      static constexpr auto hasProperty(Prop prop_id) -> bool
      {
         return s_schema.contains(prop_id);
      }

      /// @brief this gets called by TableRecord to set any missing property values
      /// 
      /// PropertyMap from the CSV file are already set, this impl just provides
      /// any calculated property values or does fixup for any parsed values that need it.
      /// 
      /// @param rec span containing a TableProperty for each PropID enum value.
      /// 
      static void onRecordParse(PropertyMap& rec)
      {
         using enum Prop;

         // set value for the WineAndVintage property
         auto vintage   = rec[Vintage ].asString();
         auto wine_name = rec[WineName].asStringView();
         rec[WineAndVintage] = ctb::format("{} {}", vintage, wine_name);

         // QtyTotal is in-stock + pending, this combined field displays similar to CT.com
         auto qty     = rec[QtyOnHand ].asUInt16().value_or(0u);
         auto pending = rec[QtyPending].asUInt16().value_or(0u);
         if (pending == 0)
         {
            rec[QtyTotal] = qty;
         }
         else{
            rec[QtyTotal] = ctb::format("{}+{}", qty, pending);
         }

         // for drinking window, 9999 = null
         auto& drink_start = rec[BeginConsume];
         if (drink_start.asUInt16() == constants::CT_NULL_YEAR)
            drink_start.setNull();

         auto& drink_end = rec[EndConsume];
         if (drink_end.asUInt16() == constants::CT_NULL_YEAR)
            drink_end.setNull();
      }

   };


   /// @brief Type alias for a CtDataTable representing the WineList table.
   ///
   using WineListTable  = CtDataTable<WineListTraits>;

} // namespace ctb