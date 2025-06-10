/**************************************************************************************************
* @file  ConsumedWineTraits.h
*
* @brief defines the ConsumedWineTraits class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
**************************************************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/table_data.h"
#include "ctb/tables/CtSchema.h"
#include "ctb/tables/detail/field_helpers.h"

#include <frozen/map.h>

namespace ctb
{
   /// @brief Traits class for a table record from the 'Pending Wine' CellarTracker CSV table.
   /// 
   class ConsumedWineTraits
   {
   public:
      using Prop             = CtProp;
      using PropertyVal      = CtPropertyVal;
      using PropType         = detail::PropType;
      using PropertyMap      = CtPropertyMap;
      using FieldSchema      = CtFieldSchema;
      using ListColumn       = CtListColumn;
      using ListColumnSpan   = CtListColumnSpan;
      using MultiValueFilter = detail::MultiValueFilter<Prop, PropertyMap>;
      using TableSort        = detail::TableSorter<CtProp, CtPropertyMap>;

      static inline constexpr auto Schema = frozen::make_map<Prop, FieldSchema>(
      {
         { Prop::iWineId,         FieldSchema { Prop::iWineId,        PropType::String,    1 }},
         { Prop::WineName,        FieldSchema { Prop::WineName,       PropType::String,   33 }},
         { Prop::Locale,          FieldSchema { Prop::Locale,         PropType::String,   35 }},
         { Prop::Vintage,         FieldSchema { Prop::Vintage,        PropType::UInt16,   32 }},
         { Prop::Country,         FieldSchema { Prop::Country,        PropType::String,   42 }},
         { Prop::Region,          FieldSchema { Prop::Region,         PropType::String,   43 }},
         { Prop::SubRegion,       FieldSchema { Prop::SubRegion,      PropType::String,   44 }},
         { Prop::Appellation,     FieldSchema { Prop::Appellation,    PropType::String,   45 }},
         { Prop::Varietal,        FieldSchema { Prop::Varietal,       PropType::String,   38 }},
         { Prop::Color,           FieldSchema { Prop::Color,          PropType::String,   36 }},
         { Prop::Category,        FieldSchema { Prop::Category,       PropType::String,   37 }},
         { Prop::MyPrice,         FieldSchema { Prop::MyPrice,        PropType::Double,   16 }},
         { Prop::Currency,        FieldSchema { Prop::Currency,       PropType::String,   17 }},
         { Prop::iTastingNoteId,  FieldSchema { Prop::iTastingNoteId, PropType::String,   20 }},
         { Prop::iConsumeId,      FieldSchema { Prop::iConsumeId,     PropType::String,    0 }},
         { Prop::ConsumeDate,     FieldSchema { Prop::ConsumeDate,    PropType::Date,      3 }},
         { Prop::ConsumeReason,   FieldSchema { Prop::ConsumeReason,  PropType::String,   11 }},
         { Prop::ConsumeNote,     FieldSchema { Prop::ConsumeNote,    PropType::String,   27 }},
         { Prop::PurchaseNote,    FieldSchema { Prop::PurchaseNote,   PropType::String,   28 }},
         { Prop::BottleNote,      FieldSchema { Prop::BottleNote,     PropType::String,   29 }},
         { Prop::Location,        FieldSchema { Prop::Location,       PropType::String,   30 }},
         { Prop::Bin,             FieldSchema { Prop::Bin,            PropType::String,   31 }},
         { Prop::Size,            FieldSchema { Prop::Size,           PropType::String,    9 }},
      });

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns{
         CtListColumn{ Prop::WineAndVintage,                             constants::DISPLAY_COL_WINE           },
         CtListColumn{ Prop::ConsumeDate,    CtListColumn::Format::Date, constants::DISPLAY_COL_CONSUME_DATE   },
         CtListColumn{ Prop::ConsumeReason,                              constants::DISPLAY_COL_CONSUME_REASON },
         CtListColumn{ Prop::Location,                                   constants::DISPLAY_COL_CONSUME_FROM   },
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts{ 
         TableSort{ { Prop::ConsumeDate, Prop::ConsumeDate, Prop::WineAndVintage }, constants::SORT_OPTION_CONSUME_DATE, true },
         TableSort{ { Prop::WineName,    Prop::Vintage                           }, constants::SORT_OPTION_WINE_VINTAGE       },
         TableSort{ { Prop::Vintage,     Prop::WineName                          }, constants::SORT_OPTION_VINTAGE_WINE       },
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiValueFilters{
         MultiValueFilter{ Prop::Vintage,     constants::FILTER_VINTAGE     },
         MultiValueFilter{ Prop::Varietal,    constants::FILTER_VARIETAL    },
         MultiValueFilter{ Prop::Country,     constants::FILTER_COUNTRY     },
         MultiValueFilter{ Prop::Region,      constants::FILTER_REGION      },
         MultiValueFilter{ Prop::Appellation, constants::FILTER_APPELATION  },
      };

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::Consumed;
      }

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableName() -> std::string_view 
      { 
         return TableDescriptions.at(getTableId());
      }

      /// @return true if the table supports the specified property, false otherwise
      static constexpr auto hasProperty(Prop prop_id) -> bool
      {
         return Schema.contains(prop_id);
      }

      /// @brief this gets called by TableRecord to set any missing property values
      /// 
      /// PropertyMap values from the CSV file are already set, this impl just provides
      /// any calculated property values or does fixup for any parsed values that need it.
      /// 
      /// @param rec - a map containing a property value for each field the table supports
      static void onRecordParse(PropertyMap& rec)
      {
         using enum Prop;

         rec[WineAndVintage] = getWineAndVintage(rec);
      }

   };

   using ConsumedWineTable = CtDataTable<ConsumedWineTraits>;

} // namespace ctb