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
   class TaggedWinesTraits
   {
   public:
      using Prop = CtProp;
      using PropertyVal = CtPropertyVal;
      using PropType = detail::PropType;
      using PropertyMap = CtPropertyMap;
      using FieldSchema = detail::FieldSchema<Prop>;
      using ListColumn = CtListColumn;
      using ListColumnSpan = CtListColumnSpan;
      using MultiValueFilter = detail::MultiValueFilter<Prop, PropertyMap>;
      using TableSort = detail::TableSorter<CtProp, CtPropertyMap>;

      static inline constexpr auto Schema = frozen::make_map<Prop, FieldSchema>(
         {
            { Prop::iWineId,         FieldSchema { Prop::iWineId,        PropType::UInt64,     11 }},
            { Prop::WineName,        FieldSchema { Prop::WineName,       PropType::String,      8 }},
            { Prop::TagName,         FieldSchema { Prop::TagName,        PropType::String,      0 }},
            { Prop::TagWineNote,     FieldSchema { Prop::TagWineNote,   PropType::String,      3 }},
            { Prop::TagMaxPrice,     FieldSchema { Prop::TagMaxPrice,    PropType::Double,      4 }},
            { Prop::Vintage,         FieldSchema { Prop::Vintage,        PropType::UInt16,      7 }},
            { Prop::Locale,          FieldSchema { Prop::Locale,         PropType::String,      9 }},
            { Prop::Producer,        FieldSchema { Prop::Producer,       PropType::String,     15 }},
            { Prop::Country,         FieldSchema { Prop::Country,        PropType::String,     21 }},
            { Prop::Region,          FieldSchema { Prop::Region,         PropType::String,     22 }},
            { Prop::SubRegion,       FieldSchema { Prop::SubRegion,      PropType::String,     23 }},
            { Prop::Appellation,     FieldSchema { Prop::Appellation,    PropType::String,     24 }},
            { Prop::Color,           FieldSchema { Prop::Color,          PropType::String,     13 }},
            { Prop::Category,        FieldSchema { Prop::Category,       PropType::String,     14 }},
            { Prop::Varietal,        FieldSchema { Prop::Varietal,       PropType::String,     18 }},
            { Prop::Size,            FieldSchema { Prop::Size,           PropType::String,      6 }},
            { Prop::WineAndVintage,  FieldSchema { Prop::WineAndVintage, PropType::String,     {} }},
         });

      /// @brief list of display columns that will show in the list view
      static inline const std::array DefaultListColumns
      {
         CtListColumn{ Prop::TagName,                                    constants::DISPLAY_COL_TAG_NAME    },
         CtListColumn{ Prop::WineAndVintage,                             constants::DISPLAY_COL_WINE        },
         CtListColumn{ Prop::Locale,                                     constants::DISPLAY_COL_LOCALE      },
      };

      /// @brief the available sort orders for this table.
      static inline const std::array AvailableSorts
      {
         TableSort{ { Prop::TagName,    Prop::WineName,   Prop::Vintage   }, constants::SORT_OPTION_TAG_WINE_VINTAGE },
         TableSort{ { Prop::TagName,    Prop::Vintage,    Prop::WineName  }, constants::SORT_OPTION_TAG_VINTAGE_WINE },
         TableSort{ { Prop::WineName,   Prop::Vintage                     }, constants::SORT_OPTION_WINE_VINTAGE     },
         TableSort{ { Prop::Vintage,    Prop::WineName                    }, constants::SORT_OPTION_VINTAGE_WINE     },
         TableSort{ { Prop::Locale,     Prop::WineName,  Prop::Vintage    }, constants::SORT_OPTION_LOCALE_WINE      },
         TableSort{ { Prop::Region,     Prop::WineName,  Prop::Vintage    }, constants::SORT_OPTION_REGION_WINE      },
      };

      /// @brief multi-value filters that can be used on this table.
      static inline const std::array MultiValueFilters
      {
         MultiValueFilter{ Prop::TagName,     constants::FILTER_TAG_NAME    },
         MultiValueFilter{ Prop::Varietal,    constants::FILTER_VARIETAL    },
         MultiValueFilter{ Prop::Vintage,     constants::FILTER_VINTAGE     },
         MultiValueFilter{ Prop::Country,     constants::FILTER_COUNTRY     },
         MultiValueFilter{ Prop::Region,      constants::FILTER_REGION      },
         MultiValueFilter{ Prop::SubRegion,   constants::FILTER_SUB_REGION  },
         MultiValueFilter{ Prop::Appellation, constants::FILTER_APPELATION  },
         MultiValueFilter{ Prop::Producer,    constants::FILTER_PRODUCER    },
      };

      /// @brief getTableName()
      /// @return the name of this CT table this traits class represents
      static constexpr auto getTableId() -> TableId
      {
         return TableId::Tag;
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
      }
   };

   using TaggedWinesTable = CtDataTable<TaggedWinesTraits>;


} // namespace ctb