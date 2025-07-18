/*******************************************************************
 * @file ListColumn.h
 *
 * @brief defines the template class ListColumn
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/PropertyValue.h"

#include <string>
#include <string_view>

namespace ctb::detail
{
   /// @brief struct containing everything needed to know about how to display a list column from a table
   ///
   template<typename PropT> requires std::is_enum_v<PropT>
   struct ListColumn
   {
      using Prop = PropT;

      /// @brief enum to specify the alignment for column headers and cell text
      ///
      /// these values align with wxWidgets' wxALIGN_xxxx values, but we don't want the dependency
      /// in the lib so just use the value directly.
      enum Align : uint16_t
      {
         Left     = 0x0000,
         Right    = 0x0200,
         Center   = 0x0900
      };

      /// @brief enum to specify the format the value will be displayed in
      enum Format
      {
         Currency,
         Date,
         Decimal,
         Number,
         String,
      };

      /// @brief The property identifer for this ListColumn
      Prop prop_id{};                  

      /// @brief Title to use for the column's header 
      std::string display_name{};

      /// @brief The format to use when displaying the value 
      Format format{ Format::String };    
      
      /// @brief How the column's values should be aligned
      Align col_align{ Align::Left };    

      /// @brief How the column header should be aligned
      Align header_align{ Align::Left }; 

      /// @brief for numeric fields, how many decimal places
      uint16_t decimal_places{ 1 };

      /// @brief construct a column to display the specified property as a string
      ListColumn(Prop prop_id, std::string_view col_name) : prop_id{ prop_id },  display_name{ col_name }
      {}

      /// @brief construct a column to display the specified property in the requested format
      ListColumn(Prop prop_id, Format fmt, std::string_view col_name, uint16_t decimal_places = 0) :  prop_id{ prop_id },  display_name{ col_name }, format{ fmt }, decimal_places{ decimal_places }
      {
         switch (format)
         {
            case Format::Currency:  [[fallthrough]];
            case Format::Decimal:   [[fallthrough]];
            case Format::Number:    [[fallthrough]];
            case Format::Date:
               col_align    = Align::Right;
               header_align = Align::Center;
               break;

            case Format::String:
               col_align    = Align::Left;
               header_align = Align::Left;
               break;

            default:
               assert("Missing enum value, this is a bug" and false);
         }
      }

      ListColumn(Prop prop_id, Format fmt, std::string_view col_name, Align col_align, Align head_align) : 
         prop_id{ prop_id },  
         display_name{ col_name }, 
         format{ fmt }, 
         col_align{ col_align },
         header_align{ head_align }
      {}

      /// @brief get the display text for a property value, which may include special formatting
      ///
      /// currency values will use a dollar sign and 2 decimal places, decimal values will use decimal_places
      template<typename... Args>
      std::string getDisplayValue(const detail::PropertyValue<Args...>& value) const
      {       
         switch (format)
         {
            case Format::Decimal:
            {
               auto fmt_str = ctb::format("{{:.{}f}}", decimal_places);
               return value.asString(fmt_str);
            }
            case Format::Currency:  
               return value.asString(constants::FMT_NUMBER_CURRENCY);

            case Format::Date:
               return value.asString(constants::FMT_DATE_SHORT);

            // regular numbers don't have special formatting except for being right-aligned.   
            case Format::Number: 
            default:                
               return value.asString();
         }
      }

      ListColumn() = default;
      ListColumn(const ListColumn&) = default;
      ListColumn(ListColumn&&) = default;
      ListColumn& operator=(const ListColumn&) = default;
      ListColumn& operator=(ListColumn&&) = default;
      ~ListColumn() = default;
   };

} // namespace ctb::detail