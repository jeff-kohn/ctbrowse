/**************************************************************************************************
* @file  TastingNotesTraits.h
*
* @brief defines the TastingNotesTraits class
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
   class TastingNotesTraits
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
         { Prop::iTastingNoteId,        FieldSchema { Prop::iTastingNoteId,       PropType::String,      0 }},
         { Prop::iWineId,               FieldSchema { Prop::iWineId,              PropType::String,      1 }},
         { Prop::WineName,              FieldSchema { Prop::WineName,             PropType::String,      5 }},
         { Prop::Locale,                FieldSchema { Prop::Locale,               PropType::String,      7 }},
         { Prop::Vintage,               FieldSchema { Prop::Vintage,              PropType::UInt16,      4 }},
         { Prop::Producer,              FieldSchema { Prop::Producer,             PropType::String,      8 }},
         { Prop::Country,               FieldSchema { Prop::Country,              PropType::String,     13 }},
         { Prop::Region,                FieldSchema { Prop::Region,               PropType::String,     14 }},
         { Prop::SubRegion,             FieldSchema { Prop::SubRegion,            PropType::String,     15 }},
         { Prop::Appellation,           FieldSchema { Prop::Appellation,          PropType::String,     16 }},
         { Prop::Color,                 FieldSchema { Prop::Color,                PropType::String,     17 }},
         { Prop::Category,              FieldSchema { Prop::Category,             PropType::String,      2 }},
         { Prop::Varietal,              FieldSchema { Prop::Varietal,             PropType::String,     10 }},
         { Prop::CtScore,               FieldSchema { Prop::CtScore,              PropType::Double,     34 }},
         { Prop::MyScore,               FieldSchema { Prop::MyScore,              PropType::Double,     25 }},
         { Prop::TastingDate,           FieldSchema { Prop::TastingDate,          PropType::Date,       18 }},
         { Prop::TastingFlawed,         FieldSchema { Prop::TastingFlawed,        PropType::Boolean,    19 }},
         { Prop::TastingLiked,          FieldSchema { Prop::TastingLiked,         PropType::Boolean,    32 }},
         { Prop::TastingNotes,          FieldSchema { Prop::TastingNotes,         PropType::String,     31 }},
         { Prop::TastingCommentCount,   FieldSchema { Prop::TastingCommentCount,  PropType::UInt16,     38 }},
         { Prop::TastingViewCount,      FieldSchema { Prop::TastingViewCount,     PropType::UInt16,     21 }},
         { Prop::TastingVoteCount,      FieldSchema { Prop::TastingVoteCount,     PropType::UInt16,     37 }},
         { Prop::TastingCtNoteCount,    FieldSchema { Prop::TastingCtNoteCount,   PropType::UInt16,     33 }},
         { Prop::TastingCtLikePercent,  FieldSchema { Prop::TastingCtLikePercent, PropType::Double,     36 }},
         { Prop::TastingCtLikeCount,    FieldSchema { Prop::TastingCtLikeCount,   PropType::UInt16,     35 }},
         { Prop::WineAndVintage,        FieldSchema { Prop::WineAndVintage,       PropType::String,     {} }},
      });

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns 
      { 
         CtListColumn{ Prop::WineAndVintage,                                  constants::DISPLAY_COL_WINE         },
         CtListColumn{ Prop::TastingDate,     CtListColumn::Format::Date,     constants::DISPLAY_COL_TASTING_DATE },
			CtListColumn{ Prop::TastingLiked,    CtListColumn::Format::Boolean,  constants::DISPLAY_COL_LIKED        },
			CtListColumn{ Prop::TastingFlawed,   CtListColumn::Format::Boolean,  constants::DISPLAY_COL_FLAWED       },
         CtListColumn{ Prop::CtScore,         CtListColumn::Format::Decimal,  constants::DISPLAY_COL_CT_SCORE, 1  },
         CtListColumn{ Prop::MyScore,         CtListColumn::Format::Decimal,  constants::DISPLAY_COL_MY_SCORE, 1  },
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts
      { 
         TableSort{ { Prop::TastingDate,  Prop::WineName, Prop::Vintage   }, constants::SORT_OPTION_TASTING_DATE, true },
         TableSort{ { Prop::WineName,     Prop::Vintage                   }, constants::SORT_OPTION_WINE_VINTAGE       },
         TableSort{ { Prop::Vintage,      Prop::WineName                  }, constants::SORT_OPTION_VINTAGE_WINE       },
         TableSort{ { Prop::MyScore,      Prop::CtScore,  Prop::WineName, }, constants::SORT_OPTION_SCORE_MY,     true },
         TableSort{ { Prop::CtScore,      Prop::MyScore,  Prop::WineName, }, constants::SORT_OPTION_SCORE_CT,     true },
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiValueFilters
      {
         MultiValueFilter{ Prop::TastingDate,       constants::FILTER_ORDER_DATE  },
         MultiValueFilter{ Prop::Varietal,          constants::FILTER_VARIETAL    },
         MultiValueFilter{ Prop::Vintage,           constants::FILTER_VINTAGE     },
         MultiValueFilter{ Prop::Country,           constants::FILTER_COUNTRY     },
         MultiValueFilter{ Prop::Region,            constants::FILTER_REGION      },
         MultiValueFilter{ Prop::SubRegion,         constants::FILTER_SUB_REGION  },
         MultiValueFilter{ Prop::Appellation,       constants::FILTER_APPELATION  },
         MultiValueFilter{ Prop::Producer,          constants::FILTER_PRODUCER    },
      };

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableId() -> TableId
      { 
         return TableId::Notes;
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
         
         auto score = rec[MyScore].asInt32().value_or(0);
         if (score == 0)
         {
            rec[MyScore].setNull();       // We don't want to see a bunch of zero's, make them blank/null.
			}
         if (rec[TastingFlawed].asBool() == false)
         { 
				rec[TastingFlawed].setNull();  // We don't want to see "No" values in the list for this property, just "Yes".
         }

         
      }

   };

   using TastingNotesTable = CtDataTable<TastingNotesTraits>;

} // namespace ctb