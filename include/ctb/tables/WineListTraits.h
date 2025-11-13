/**************************************************************************************************
* @file  WineListTraits.h
*
* @brief defines the WineListTraits class, which is an instantiation of CtDataTable<> 
*        implemented using the traits template WineListTraits 
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
   /// @brief Traits class for a table record from the 'List' CellarTracker CSV table.
   /// 
   class WineListTraits
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
         { Prop::iWineId,         FieldSchema { Prop::iWineId,        PropType::UInt64,      0 }},
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
         { Prop::QtyPurchased,    FieldSchema { Prop::QtyPurchased,   PropType::UInt16,     13 }},
         { Prop::QtyConsumed,     FieldSchema { Prop::QtyConsumed,    PropType::UInt16,     19 }},
         { Prop::Size,            FieldSchema { Prop::Size,           PropType::String,      4 }},
         { Prop::BeginConsume,    FieldSchema { Prop::BeginConsume,   PropType::UInt16,     63 }},
         { Prop::EndConsume,      FieldSchema { Prop::EndConsume,     PropType::UInt16,     64 }},
         { Prop::MyPrice,         FieldSchema { Prop::MyPrice,        PropType::Double,      5 }},
         { Prop::CtPrice,         FieldSchema { Prop::CtPrice,        PropType::Double,      9 }},
         { Prop::AuctionPrice,    FieldSchema { Prop::AuctionPrice,   PropType::Double,      8 }},
         { Prop::WineAndVintage,  FieldSchema { Prop::WineAndVintage, PropType::String,     {} }},
         { Prop::QtyTotal,        FieldSchema { Prop::QtyTotal,       PropType::String,     {} }},
      });

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns 
      { 
         CtListColumn{ Prop::WineAndVintage,                             constants::DISPLAY_COL_WINE        },
         CtListColumn{ Prop::Locale,                                     constants::DISPLAY_COL_LOCALE      },
         CtListColumn{ Prop::QtyTotal,   CtListColumn::Format::Number,   constants::DISPLAY_COL_QTY         },
         CtListColumn{ Prop::CtScore,    CtListColumn::Format::Decimal,  constants::DISPLAY_COL_CT_SCORE, 1 },
         CtListColumn{ Prop::MyScore,    CtListColumn::Format::Decimal,  constants::DISPLAY_COL_MY_SCORE, 1 },
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts 
      { 
         TableSort{ { Prop::WineName, Prop::Vintage                       }, constants::SORT_OPTION_WINE_VINTAGE   },
         TableSort{ { Prop::Vintage,  Prop::WineName                      }, constants::SORT_OPTION_VINTAGE_WINE   },
         TableSort{ { Prop::Locale,   Prop::WineName,    Prop::Vintage    }, constants::SORT_OPTION_LOCALE_WINE    },
         TableSort{ { Prop::Region,   Prop::WineName,    Prop::Vintage    }, constants::SORT_OPTION_REGION_WINE    },
         TableSort{ { Prop::CtScore,  Prop::MyScore,     Prop::WineName,  }, constants::SORT_OPTION_SCORE_CT, true },
         TableSort{ { Prop::MyScore,  Prop::CtScore,     Prop::WineName,  }, constants::SORT_OPTION_SCORE_MY, true },
         TableSort{ { Prop::MyPrice,  Prop::WineName,    Prop::Vintage ,  }, constants::SORT_OPTION_MY_VALUE       }
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiValueFilters 
      {
         MultiValueFilter{ Prop::Varietal,    constants::FILTER_VARIETAL    },
         MultiValueFilter{ Prop::Vintage,     constants::FILTER_VINTAGE     },
         MultiValueFilter{ Prop::Country,     constants::FILTER_COUNTRY     },
         MultiValueFilter{ Prop::Region,      constants::FILTER_REGION      },
         MultiValueFilter{ Prop::SubRegion,   constants::FILTER_SUB_REGION  },
         MultiValueFilter{ Prop::Appellation, constants::FILTER_APPELATION  },
         MultiValueFilter{ Prop::Producer,    constants::FILTER_PRODUCER    },
         MultiValueFilter{ Prop::Size,        constants::FILTER_BOTTLE_SIZE },
      };

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::List; 
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
      /// @param rec - map containing a PropertyValue for each PropID enum value.
      static void onRecordParse(PropertyMap& rec)
      {
         using enum Prop;

         rec[WineAndVintage] = getWineAndVintage(rec);
         rec[QtyTotal]       = calcQtyTotal(rec);

         validateDrinkYear(rec[BeginConsume]);
         validateDrinkYear(rec[EndConsume]);
      }
   };

   using WineListTable = CtDataTable<WineListTraits>;


} // namespace ctb