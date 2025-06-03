/**************************************************************************************************
* @file  ReadyToDrinkTraits.h
*
* @brief defines the WineListTable class, which is an instantiation of CtDataTable<> 
*        implemented using the traits template ReadyToDrinkTraits 
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
   class ReadyToDrinkTraits
   {
   public:
      using Prop                 = CtProp;
      using Property             = CtProperty;
      using PropType             = detail::PropType;
      using PropertyMap          = CtPropertyMap;
      using FieldSchema          = detail::FieldSchema<Prop>;
      using ListColumn           = CtListColumn;
      using ListColumnSpan       = CtListColumnSpan;
      using MultiMatchFilter     = detail::MultiMatchPropertyFilter<Prop, PropertyMap>;
      using TableSort            = detail::TableSorter<CtProp, CtPropertyMap>;

      static inline constexpr auto Schema = frozen::make_map<Prop, FieldSchema>(
      {
         { Prop::WineAndVintage,       FieldSchema { Prop::WineAndVintage,       PropType::Double,   {} }},
         { Prop::iWineId,              FieldSchema { Prop::iWineId,              PropType::String,    0 }},
         { Prop::WineName,             FieldSchema { Prop::WineName,             PropType::String,   23 }},
         { Prop::Locale,               FieldSchema { Prop::Locale,               PropType::String,   25 }},
         { Prop::Vintage,              FieldSchema { Prop::Vintage,              PropType::UInt16,   22 }},
         { Prop::Producer,             FieldSchema { Prop::Producer,             PropType::String,   26 }},
         { Prop::Country,              FieldSchema { Prop::Country,              PropType::String,   31 }},
         { Prop::Region,               FieldSchema { Prop::Region,               PropType::String,   32 }},
         { Prop::SubRegion,            FieldSchema { Prop::SubRegion,            PropType::String,   33 }},
         { Prop::Appellation,          FieldSchema { Prop::Appellation,          PropType::String,   34 }},
         { Prop::Color,                FieldSchema { Prop::Color,                PropType::String,    2 }},
         { Prop::Category,             FieldSchema { Prop::Category,             PropType::String,    3 }},
         { Prop::Varietal,             FieldSchema { Prop::Varietal,             PropType::String,   28 }},
         { Prop::CtScore,              FieldSchema { Prop::CtScore,              PropType::Double,  174 }},
         { Prop::MyScore,              FieldSchema { Prop::MyScore,              PropType::Double,  171 }},
         { Prop::QtyOnHand,            FieldSchema { Prop::QtyOnHand,            PropType::UInt16,   16 }},
         { Prop::QtyPending,           FieldSchema { Prop::QtyPending,           PropType::UInt16,   15 }},
         { Prop::QtyTotal,             FieldSchema { Prop::QtyTotal,             PropType::UInt16,   21 }},
         { Prop::QtyConsumed,          FieldSchema { Prop::QtyConsumed,          PropType::UInt16,   19 }},
         { Prop::QtyPurchased,         FieldSchema { Prop::QtyPurchased,         PropType::UInt16,   13 }},
         { Prop::BeginConsume,         FieldSchema { Prop::BeginConsume,         PropType::UInt16,   35 }},
         { Prop::EndConsume,           FieldSchema { Prop::EndConsume,           PropType::UInt16,   36 }},
         { Prop::CtBeginConsume,       FieldSchema { Prop::CtBeginConsume,       PropType::UInt16,   63 }},
         { Prop::CtEndConsume,         FieldSchema { Prop::CtEndConsume,         PropType::UInt16,   64 }},
         { Prop::RtdQtyDefault,        FieldSchema { Prop::RtdQtyDefault,        PropType::Double,    4 }},
         { Prop::RtdQtyLinear,         FieldSchema { Prop::RtdQtyLinear,         PropType::Double,    5 }},
         { Prop::RtdQtyBellCurve,      FieldSchema { Prop::RtdQtyBellCurve,      PropType::Double,    6 }},
         { Prop::RtdQtyEarlyCurve,     FieldSchema { Prop::RtdQtyEarlyCurve,     PropType::Double,    7 }},
         { Prop::RtdQtyLateCurve,      FieldSchema { Prop::RtdQtyLateCurve,      PropType::Double,    8 }},
         { Prop::RtdQtyFastMaturing,   FieldSchema { Prop::RtdQtyFastMaturing,   PropType::Double,    9 }},
         { Prop::RtdQtyEarlyAndLate,   FieldSchema { Prop::RtdQtyEarlyAndLate,   PropType::Double,   10 }},
         { Prop::RtdQtyBottlesPerYear, FieldSchema { Prop::RtdQtyBottlesPerYear, PropType::Double,   11 }},
         { Prop::RtdConsumed,          FieldSchema { Prop::RtdConsumed,          PropType::String,   {} }},
      });

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns { 
         CtListColumn{ Prop::WineAndVintage,     CtListColumn::Format::String,  constants::DISPLAY_COL_WINE             },
         CtListColumn{ Prop::RtdConsumed,        CtListColumn::Format::String,  constants::DISPLAY_COL_PURCHASES, ListColumn::Align::Right, ListColumn::Align::Center },
         CtListColumn{ Prop::RtdQtyDefault,      CtListColumn::Format::Decimal, constants::DISPLAY_COL_AVAILABLE,     2 },
         CtListColumn{ Prop::RtdQtyLinear,       CtListColumn::Format::Decimal, constants::DISPLAY_COL_LINEAR,        2 },
         CtListColumn{ Prop::RtdQtyBellCurve,    CtListColumn::Format::Decimal, constants::DISPLAY_COL_BELL_CURVE ,   2 },
         CtListColumn{ Prop::RtdQtyEarlyCurve,   CtListColumn::Format::Decimal, constants::DISPLAY_COL_EARLY_CURVE,   2 },
         CtListColumn{ Prop::RtdQtyLateCurve,    CtListColumn::Format::Decimal, constants::DISPLAY_COL_LATE_CURVE,    2 },
         CtListColumn{ Prop::RtdQtyEarlyAndLate, CtListColumn::Format::Decimal, constants::DISPLAY_COL_EARLY_LATE,    2 },
         CtListColumn{ Prop::RtdQtyFastMaturing, CtListColumn::Format::Decimal, constants::DISPLAY_COL_FAST_MATURING, 2 },
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts{ 
         TableSort{ { Prop::RtdQtyDefault,        Prop::WineName,  }, constants::SORT_OPTION_CURVE_DEFAULT,      true },
         TableSort{ { Prop::RtdQtyLinear,         Prop::WineName,  }, constants::SORT_OPTION_CURVE_LINEAR,       true },
         TableSort{ { Prop::RtdQtyBellCurve,      Prop::WineName,  }, constants::SORT_OPTION_CURVE_BELL,         true },
         TableSort{ { Prop::RtdQtyEarlyCurve,     Prop::WineName,  }, constants::SORT_OPTION_CURVE_BELL_EARLY,   true },
         TableSort{ { Prop::RtdQtyLateCurve,      Prop::WineName,  }, constants::SORT_OPTION_CURVE_BELL_LATE,    true },
         TableSort{ { Prop::RtdQtyFastMaturing,   Prop::WineName,  }, constants::SORT_OPTION_CURVE_FAST_MATURE,  true },
         TableSort{ { Prop::RtdQtyEarlyAndLate,   Prop::WineName,  }, constants::SORT_OPTION_CURVE_EARLY_LATE,   true },
         TableSort{ { Prop::RtdQtyBottlesPerYear, Prop::WineName,  }, constants::SORT_OPTION_CURVE_BOTTLES_YEAR, true },
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiMatchFilters{
         MultiMatchFilter{ Prop::Varietal,    constants::FILTER_VARIETAL   },
         MultiMatchFilter{ Prop::Vintage,     constants::FILTER_VINTAGE    },
         MultiMatchFilter{ Prop::Country,     constants::FILTER_COUNTRY    },
         MultiMatchFilter{ Prop::Region,      constants::FILTER_REGION     },
         MultiMatchFilter{ Prop::Appellation, constants::FILTER_APPELATION },
         MultiMatchFilter{ Prop::Producer,    constants::FILTER_PRODUCER   },
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
         return TableDescriptions.at(getTableId());
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
      /// @param rec - map containing a TableProperty for each PropID enum value.
      static void onRecordParse(PropertyMap& rec)
      {
         using enum Prop;

         rec[WineAndVintage] = getWineAndVintage(rec);
         rec[QtyTotal]       = calcQtyTotal(rec);
         rec[RtdConsumed]    = getRtdConsumed(rec);

         validateDrinkYear(rec[EndConsume]);
         validateDrinkYear(rec[CtBeginConsume]);
         validateDrinkYear(rec[CtEndConsume]);
      }
   };

   using ReadyToDrinkTable = CtDataTable<ReadyToDrinkTraits>;


} // namespace ctb