/*******************************************************************
* @file  WineListTraits.h
*
* @brief defines the traits template class WineListTraits which is 
*        used in conjunction with CtRecordImpl to implement the
*        'List' CellarTracker table.
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/table/CtRecordImpl.h"

#include <frozen/map.h>

//#include <utility>
#include <span>
#include <vector>

namespace ctb
{
   /// @brief class representing a table record from the 'List' CellarTracker CSV table.
   ///
   /// since our CRTP base doesn't implement any interfaces and is just providing implementation
   /// we can use protected inheritance
   /// 
   class WineListTraits
   {
   public:
      /// @brief these are the fields that this object contains properties for.
      /// 
      enum class PropId : uint16_t
      {
         iWineId,
         WineName,
         Locale,
         Vintage,
         Quantity,
         Pending,
         Size,
         Price,
         AuctionPrice,
         CtPrice,
         Country,
         Region,
         SubRegion,
         Appellation,
         Producer,
         SortProducer,
         Color,
         Category,
         MasterVarietal,
         CTScore,
         MYScore,
         BeginConsume,
         EndConsume,
         WineAndVintage,
         TotalQty
      };

      /// @brief - contains the list of data fields that are parsed from CSV
      /// 
      /// this collection contains the list of properties that we actually parse from the CSV file. Any
      /// calculated properties are not included here which explains why this array doesn't contain 
      /// every PropId enum value.
      /// 
      static inline constexpr frozen::map<PropId, FieldSchema, static_cast<size_t>(PropId::WineAndVintage)> CsvSchema
      {
         { PropId::iWineId,         FieldSchema { static_cast<uint32_t>(PropId::iWineId),        PropType::String,      0 }},
         { PropId::WineName,        FieldSchema { static_cast<uint32_t>(PropId::WineName),       PropType::String,     13 }},
         { PropId::Locale,          FieldSchema { static_cast<uint32_t>(PropId::Locale),         PropType::String,     14 }},
         { PropId::Vintage,         FieldSchema { static_cast<uint32_t>(PropId::Vintage),        PropType::UInt16,     12 }},
         { PropId::Quantity,        FieldSchema { static_cast<uint32_t>(PropId::Quantity),       PropType::UInt16,      2 }},
         { PropId::Pending,         FieldSchema { static_cast<uint32_t>(PropId::Pending),        PropType::UInt16,      3 }},
         { PropId::Size,            FieldSchema { static_cast<uint32_t>(PropId::Size),           PropType::String,      4 }},
         { PropId::Price,           FieldSchema { static_cast<uint32_t>(PropId::Price),          PropType::Double,      5 }},
         { PropId::AuctionPrice,    FieldSchema { static_cast<uint32_t>(PropId::AuctionPrice),   PropType::Double,      8 }},
         { PropId::CtPrice,         FieldSchema { static_cast<uint32_t>(PropId::CtPrice),        PropType::Double,      9 }},
         { PropId::Country,         FieldSchema { static_cast<uint32_t>(PropId::Country),        PropType::String,     15 }},
         { PropId::Region,          FieldSchema { static_cast<uint32_t>(PropId::Region),         PropType::String,     16 }},
         { PropId::SubRegion,       FieldSchema { static_cast<uint32_t>(PropId::SubRegion),      PropType::String,     17 }},
         { PropId::Appellation,     FieldSchema { static_cast<uint32_t>(PropId::Appellation),    PropType::String,     18 }},
         { PropId::Producer,        FieldSchema { static_cast<uint32_t>(PropId::Producer),       PropType::String,     19 }},
         { PropId::SortProducer,    FieldSchema { static_cast<uint32_t>(PropId::SortProducer),   PropType::String,     20 }},
         { PropId::Color,           FieldSchema { static_cast<uint32_t>(PropId::Color),          PropType::String,     22 }},
         { PropId::Category,        FieldSchema { static_cast<uint32_t>(PropId::Category),       PropType::String,     23 }},
         { PropId::MasterVarietal,  FieldSchema { static_cast<uint32_t>(PropId::MasterVarietal), PropType::String,     25 }},
         { PropId::CTScore,         FieldSchema { static_cast<uint32_t>(PropId::CTScore),        PropType::Double,     59 }},
         { PropId::MYScore,         FieldSchema { static_cast<uint32_t>(PropId::MYScore),        PropType::Double,     61 }},
         { PropId::BeginConsume,    FieldSchema { static_cast<uint32_t>(PropId::BeginConsume),   PropType::UInt16,     63 }},
         { PropId::EndConsume,      FieldSchema { static_cast<uint32_t>(PropId::EndConsume),     PropType::UInt16,     64 }}
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
         return "WineList"; 
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
         return true;
      }

      /// @brief this gets called by CtRecordImpl to set any missing property values
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

         // total qty is in-stock + pending, this combined field displays similar to CT.com
         auto qty     = rec[static_cast<size_t>(PropId::Quantity)].asUInt16().value_or(0u);
         auto pending = rec[static_cast<size_t>(PropId::Pending) ].asUInt16().value_or(0u);
         if (pending == 0)
         {
            rec[static_cast<size_t>(PropId::TotalQty)] = qty;
         }
         else{
            rec[static_cast<size_t>(PropId::TotalQty)] = ctb::format("{}+{}", qty, pending);
         }

         // for drinking window, 9999 = null
         auto& drink_start = rec[static_cast<size_t>(PropId::BeginConsume)];
         if (drink_start.asUInt16() == constants::CT_NULL_YEAR)
            drink_start.setNull();

         auto& drink_end = rec[static_cast<size_t>(PropId::EndConsume)];
         if (drink_end.asUInt16() == constants::CT_NULL_YEAR)
            drink_end.setNull();
      }
   };

   using WineListRecord    = CtRecordImpl<WineListTraits>;
   using WineListDataset   = CtDataset<WineListTraits>;

} // namespace ctb