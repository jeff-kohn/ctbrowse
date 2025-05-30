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
#include "ctb/tables/CtSchema.h"

#include <frozen/map.h>

namespace ctb
{

   /// @brief Traits class for a table record from the 'Pending Wine' CellarTracker CSV table.
   /// 
   class PendingWineTraits
   {
   public:
      using Prop             = CtProp;
      using Property         = CtProperty;
      using PropType         = detail::PropType;
      using PropertyMap      = CtPropertyMap;
      using FieldSchema      = CtFieldSchema;
      using ListColumn       = CtListColumn;
      using ListColumnSpan   = CtListColumnSpan;
      using MultiMatchFilter = detail::MultiMatchPropertyFilter<Prop, PropertyMap>;
      using TableSort        = detail::TableSorter<CtProp, CtPropertyMap>;

      static inline constexpr auto Schema = frozen::make_map<Prop, FieldSchema>(
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
         { Prop::PendingOrderDate,    FieldSchema { Prop::PendingOrderDate,   PropType::Date,        2 }}, 
         { Prop::PendingDeliveryDate,    FieldSchema { Prop::PendingDeliveryDate,   PropType::Date,        3 }},
         { Prop::WineAndVintage,         FieldSchema { Prop::WineAndVintage,        PropType::Double,     {} }}
      });

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns { 
         CtListColumn{ Prop::WineAndVintage,                                      constants::DISPLAY_COL_WINE       },
         CtListColumn{ Prop::PendingStoreName,    CtListColumn::Format::String,   constants::DISPLAY_COL_STORE      },
         CtListColumn{ Prop::PendingOrderDate,    CtListColumn::Format::Date,     constants::DISPLAY_COL_PURCH_DATE },
         CtListColumn{ Prop::PendingQtyOrdered,   CtListColumn::Format::Number,   constants::DISPLAY_COL_QTY        },
         CtListColumn{ Prop::PendingPrice,        CtListColumn::Format::Currency, constants::DISPLAY_COL_PRICE      }
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts{ 
         TableSort{ { Prop::PendingOrderDate,    Prop::WineName, Prop::Vintage  }, constants::SORT_OPTION_PURCHASE_DATE },
         TableSort{ { Prop::WineName,            Prop::Vintage                  }, constants::SORT_OPTION_WINE_VINTAGE  },
         TableSort{ { Prop::Vintage,             Prop::WineName                 }, constants::SORT_OPTION_VINTAGE_WINE  },
         TableSort{ { Prop::PendingStoreName,    Prop::WineName, Prop::Vintage, }, constants::SORT_OPTION_STORE_NAME    }
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiMatchFilters{
         MultiMatchFilter{ Prop::Varietal,          constants::FILTER_VARIETAL    },
         MultiMatchFilter{ Prop::Country,           constants::FILTER_COUNTRY     },
         MultiMatchFilter{ Prop::Region,            constants::FILTER_REGION      },
         MultiMatchFilter{ Prop::Appellation,       constants::FILTER_APPELATION  },
         MultiMatchFilter{ Prop::Vintage,           constants::FILTER_VINTAGE     },
         MultiMatchFilter{ Prop::PendingStoreName,  constants::FILTER_STORE       },
         MultiMatchFilter{ Prop::PendingOrderDate,  constants::FILTER_ORDER_DATE  }
      };

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::Pending;
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

         // set value for the WineAndVintage property
         auto vintage   = rec[Vintage ].asString();
         auto wine_name = rec[WineName].asStringView();
         rec[WineAndVintage] = ctb::format("{} {}", vintage, wine_name);

         // CT defaults delivery date to order date if you don't fill it in, we don't want to display that
         // (website doesn't either.
         if (rec[PendingOrderDate] == rec[PendingDeliveryDate])
         {
            rec[PendingDeliveryDate] = ct_null_prop;
         }
      }
      
   };

   using PendingWineTable = CtDataTable<PendingWineTraits>;

} // namespace ctb