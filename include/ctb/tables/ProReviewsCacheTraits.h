/**************************************************************************************************
* @file  ProReviewsCacheTraits.h
*
* @brief defines the WineListTable class, which is an instantiation of CtDataTable<> 
*        implemented using the traits template ProReviewsCacheTraits 
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
**************************************************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/table_data.h"
#include "ctb/tables/CtSchema.h"
#include "ctb/tables/detail/field_helpers.h"

#include <frozen/map.h>
#include <array>
#include <string_view>

namespace ctb
{
   /// @brief Traits class for a table record from the 'Availability' CellarTracker CSV table.
   /// 
   class ProReviewsCacheTraits
   {
   public:
      using Prop                 = CtProp;
      using PropertyVal          = CtPropertyVal;
      using PropType             = detail::PropType;
      using PropertyMap          = CtPropertyMap;
      using FieldSchema          = detail::FieldSchema<Prop>;
      using ListColumn           = CtListColumn;
      using ListColumnSpan       = CtListColumnSpan;
      using MultiValueFilter     = detail::MultiValueFilter<Prop, PropertyMap>;
      using TableSort            = detail::TableSorter<CtProp, CtPropertyMap>;

      static inline constexpr auto Schema = frozen::make_map<Prop, FieldSchema>(
      {
         { Prop::iWineId,              FieldSchema { Prop::iWineId,              PropType::UInt64,    0 }},
         { Prop::CtScore,              FieldSchema { Prop::CtScore,              PropType::Double,  174 }},
         { Prop::MyScore,              FieldSchema { Prop::MyScore,              PropType::Double,  171 }},
         { Prop::BeginConsume,         FieldSchema { Prop::BeginConsume,         PropType::UInt16,   35 }},
         { Prop::EndConsume,           FieldSchema { Prop::EndConsume,           PropType::UInt16,   36 }},
         { Prop::CtBeginConsume,       FieldSchema { Prop::CtBeginConsume,       PropType::UInt16,   63 }},
         { Prop::CtEndConsume,         FieldSchema { Prop::CtEndConsume,         PropType::UInt16,   64 }},
         { Prop::DR_ScoreDisplay,      FieldSchema { Prop::DR_ScoreDisplay,      PropType::String,  152 }},
         { Prop::DR_ScoreNumeric,      FieldSchema { Prop::DR_ScoreNumeric,      PropType::Double,  154 }},
         { Prop::DR_DrinkBegin,        FieldSchema { Prop::DR_DrinkBegin,        PropType::UInt16,   53 }},
         { Prop::DR_DrinkEnd,          FieldSchema { Prop::DR_DrinkEnd,          PropType::UInt16,   54 }},
         { Prop::JD_ScoreDisplay,      FieldSchema { Prop::JD_ScoreDisplay,      PropType::String,  128 }},
         { Prop::JD_ScoreNumeric,      FieldSchema { Prop::JD_ScoreNumeric,      PropType::Double,  130 }},
         //{ Prop::JD_DrinkBegin,        FieldSchema { Prop::JD_DrinkBegin,        PropType::UInt16,      }},
         //{ Prop::JD_DrinkEnd,          FieldSchema { Prop::JD_DrinkEnd,          PropType::UInt16,      }},
         { Prop::JR_ScoreDisplay,      FieldSchema { Prop::JR_ScoreDisplay,      PropType::String,  110 }},
         { Prop::JR_ScoreNumeric,      FieldSchema { Prop::JR_ScoreNumeric,      PropType::Double,  112 }},
         { Prop::JR_DrinkBegin,        FieldSchema { Prop::JR_DrinkBegin,        PropType::UInt16,   51 }},
         { Prop::JR_DrinkEnd,          FieldSchema { Prop::JR_DrinkEnd,          PropType::UInt16,   52 }},
         { Prop::WFW_ScoreDisplay,     FieldSchema { Prop::WFW_ScoreDisplay,     PropType::String,  113 }},
         { Prop::WFW_ScoreNumeric,     FieldSchema { Prop::WFW_ScoreNumeric,     PropType::Double,  115 }},
         //{ Prop::WFW_DrinkBegin,       FieldSchema { Prop::WFW_DrinkBegin,       PropType::UInt16,      }},
         //{ Prop::WFW_DrinkEnd,         FieldSchema { Prop::WFW_DrinkEnd,         PropType::UInt16,      }},
      });

      static inline constexpr int TWO_DECIMAL_PLACES{ 2 };

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns
      {
         ListColumn{}
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts
      { 
         TableSort{}
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiValueFilters
      {
         MultiValueFilter{}
      };

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::Availability;
      }

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableName() -> std::string_view 
      { 
         return "RTD_Review_Cache";
      }

      /// @brief hasProperty()
      /// @return true if the table supports the specified property, false otherwise
      static constexpr auto hasProperty(Prop prop_id) -> bool
      {
         return Schema.contains(prop_id);
      }

      /// @brief this gets called by TableRecord to set any missing property values
      /// 
      /// PropertyMap from the CSV file are already set, this impl just provides
      /// any calculated property values or does fixup for any parsed values that need it.
      /// 
      /// @param rec - map containing a PropertyValue for each PropID enum value.
      static void onRecordParse(PropertyMap& rec)
      {
         using enum Prop;

         validateDrinkYear(rec[BeginConsume]);
         validateDrinkYear(rec[EndConsume]);
         validateDrinkYear(rec[CtBeginConsume]);
         validateDrinkYear(rec[CtEndConsume]);
      }
   };

   using ProReviewsCacheTable = CtDataTable<ProReviewsCacheTraits>;


} // namespace ctb