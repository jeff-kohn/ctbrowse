/*******************************************************************
* @file  WineListTraits.h
*
* @brief defines the traits template class WineListTraits which is 
*        used in conjuction with CtRecordImpl to implement the
*        'List' CellarTracker table.
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "CtRecordImpl.h"

#include <frozen/map.h>

#include <format>
#include <utility>
#include <span>

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
      enum class PropId
      {
         iWineID,
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
         WineAndVintage
      };


      /// @brief - contains the list of data fields that are parsed from CSV
      /// 
      /// this collection contains the list of properties that we actually parse from the CSV file. Any
      /// calculated properties are not included here which explains why this array doesn't contain 
      /// every PropId enum value.
      /// 
      static inline constexpr frozen::map<PropId, FieldSchema, static_cast<size_t>(PropId::EndConsume)> CsvSchema
      {
         { PropId::iWineID,         FieldSchema { static_cast<uint32_t>(PropId::iWineID),        PropType::String,      0 }},
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

      static const auto& getCsvSchema() { return CsvSchema; }


      /// @brief small helper to convert a Prop enum into its integer index
      /// 
      static constexpr int propToIndex(PropId prop)
      {
         return enumToIndex(prop);
      }


      /// @brief small helper to convert a zero-based index to a Prop enum
      /// 
      static constexpr PropId propFromIndex(int idx)
      {
         return enumFromIndex<PropId>(idx);
      }

      /// @brief this gets called by CtRecordImpl to get the value for calculated properties not in the CSV
      ///
      template<std::size_t N, typename... Args>
      static void getCalculatedValue(std::span<TableProperty<Args...>, N> rec, PropId prop_id)
      {
         using enum PropId;

         switch (prop_id)
         {
            case PropId::WineAndVintage:
            {
               auto vintage   = rec[static_cast<size_t>(Vintage) ].asString();
               auto wine_name = rec[static_cast<size_t>(WineName)].asString();
               rec[static_cast<size_t>(prop_id)] = std::format("{} {}", vintage, wine_name);
               break;
            }
            default:
               assert("Unexpected PropId passed to WineListTraits::getCalculatedValue()");
         }
      }
   };

   using WineListRecord = CtRecordImpl<WineListTraits>;
   using WineListData   = std::deque<WineListRecord>;

} // namespace ctb