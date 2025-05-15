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
#include "ctb/table_data.h"
#include "ctb/tables/CtDataTable.h"

#include <frozen/map.h>


namespace ctb
{

   /// @brief Traits class for a table record from the 'Pending Wine' CellarTracker CSV table.
   /// 
   class PendingWineTraits
   {
   public:
      using Prop        = CtProp;
      using Property    = CtProperty;
      using PropertyMap = CtPropertyMap;
      using FieldSchema = FieldSchema;

   private:
      static inline constexpr auto s_schema = frozen::make_map<Prop, FieldSchema>(
      {
         { Prop::iWineId,                FieldSchema { Prop::iWineId,               PropType::String,      0 }},
         { Prop::WineName,               FieldSchema { Prop::WineName,              PropType::String,     17 }},
         { Prop::Locale,                 FieldSchema { Prop::Locale,                PropType::String,     19 }},
         { Prop::Vintage,                FieldSchema { Prop::Vintage,               PropType::UInt16,     16 }},
         { Prop::Producer,               FieldSchema { Prop::Producer,              PropType::String,     23 }},
         { Prop::Country,                FieldSchema { Prop::Country,               PropType::String,     28 }},
         { Prop::Region,                 FieldSchema { Prop::Region,                PropType::String,     29 }},
         { Prop::SubRegion,              FieldSchema { Prop::SubRegion,             PropType::String,     30 }},
         { Prop::Appellation,            FieldSchema { Prop::Appellation,           PropType::String,     31 }},
         { Prop::Color,                  FieldSchema { Prop::Color,                 PropType::String,     21 }},
         { Prop::Category,               FieldSchema { Prop::Category,              PropType::String,     22 }},
         { Prop::Varietal,               FieldSchema { Prop::Varietal,              PropType::String,     25 }},
         { Prop::QtyPending,             FieldSchema { Prop::QtyPending,            PropType::UInt16,     11 }},
         { Prop::Size,                   FieldSchema { Prop::Size,                  PropType::String,     14 }},
         { Prop::Currency,               FieldSchema { Prop::Currency,              PropType::String,      5 }},
         { Prop::PendingPrice,           FieldSchema { Prop::PendingPrice,          PropType::Double,      7 }},
         { Prop::PendingPurchaseId,      FieldSchema { Prop::PendingPurchaseId,     PropType::String,      1 }},
         { Prop::PendingStoreName,       FieldSchema { Prop::PendingStoreName,      PropType::String,      4 }},
         { Prop::PendingOrderNumber,     FieldSchema { Prop::PendingOrderNumber,    PropType::String,     12 }},
         { Prop::PendingQtyOrdered,      FieldSchema { Prop::PendingQtyOrdered,     PropType::UInt16,     10 }},
         { Prop::PendingPurchaseDate,    FieldSchema { Prop::PendingPurchaseDate,   PropType::Date,        2 }}, 
         { Prop::PendingDeliveryDate,    FieldSchema { Prop::PendingDeliveryDate,   PropType::Date,        3 }},
         { Prop::WineAndVintage,         FieldSchema { Prop::WineAndVintage,        PropType::Double,     {} }},
      });

   public:
      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      /// 
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::Pending;
      }

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      /// 
      static constexpr auto getTableName() -> std::string_view 
      { 
         return TableDescriptions.at(getTableId());
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
      /// PropertyMap values from the CSV file are already set, this impl just provides
      /// any calculated property values or does fixup for any parsed values that need it.
      /// 
      /// @param rec - a map containing a property value for each field the table supports
      /// 
      static void onRecordParse(PropertyMap& rec)
      {
         using enum Prop;

         // set value for the WineAndVintage property
         auto vintage   = rec[Vintage ].asString();
         auto wine_name = rec[WineName].asStringView();
         rec[WineAndVintage] = ctb::format("{} {}", vintage, wine_name);
      }
   };


   /// @brief Type alias for a CtDataTable representing the PendingWines table.
   ///
   using PendingWineTable  = CtDataTable<PendingWineTraits>;

} // namespace ctb