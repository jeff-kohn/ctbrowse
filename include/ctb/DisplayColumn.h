/*******************************************************************
 * @file DisplayColumn.h
 *
 * @brief defines the template class DisplayColumn
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/functors.h"
#include "ctb/TableProperty.h"
#include "ctb/table_data.h"

#include <magic_enum/magic_enum.hpp>
#include <wx/defs.h>

#include <variant>

namespace ctb
{

   /// @brief struct containing everything needed to know about how to display a table column
   ///
   template<CtRecord RecordType>
   struct DisplayColumn
   {
      // some types we borrow from our template parameter
      using PropId         = RecordType::PropId;
      using TableProperty  = RecordType::TableProperty;

      /// @brief enum to specify the alignment for column headers and cell text
      ///
      enum Align : uint16_t
      {
         Left = wxAlignment::wxALIGN_LEFT,
         Right = wxAlignment::wxALIGN_RIGHT,
         Center = wxAlignment::wxALIGN_CENTER
      };


      /// @brief enum to specify the format the value will be displayed in
      ///
      enum Format
      {
         String,
         Number,
         Decimal,
         Currency
      };


      /// properties
      ///
      PropId       prop_id{};                   /// identifier for the property to display
      std::string  display_name{};              /// column header title to use
      Format       format{ Format::String };    /// format to display the value
      Align        col_align{ Align::Left };    /// how to align the column values in the cell
      Align        header_align{ Align::Left }; /// how to align the column title in the header


      /// @brief construct a column to display the specified property as a string
      ///
      /// column header value is option and will use the table column name by default
      ///
      explicit DisplayColumn(PropId prop, std::string_view col_name = {}) : prop_id{ prop }, display_name{ col_name }
      {
         if (display_name.empty())
         {
            display_name = magic_enum::enum_name(prop);
         }
      }


      /// @brief construct a column to display the specified property in the requested format
      ///
      /// column header value is option and will use the table column name by default
      ///
      DisplayColumn(PropId prop, Format fmt, std::string_view col_name = {}) : prop_id{ prop }, display_name{ col_name }, format{ fmt }
      {
         if (display_name.empty())
         {
            display_name = magic_enum::enum_name(prop);
         }

         if (fmt != Format::String)
         {
            col_align = Align::Right;
            header_align = Align::Center;
         }
      }


      /// @brief get the display text for a property value, which may include special formatting
      ///
      /// currency values will use a dollar sign and 2 decimal places, decimal values  will be 
      /// displayed with 1 decimal place.
      ///
      std::string getDisplayValue(const TableProperty& value) const
      {       
         switch (format)
         {
            case Format::Decimal:   
               return value.asString(constants::FMT_NUMBER_DECIMAL);

            case Format::Currency:  
               return value.asString(constants::FMT_NUMBER_CURRENCY);

            case Format::Number: // regular numbers don't have special formatting excpept for being right-aligned.   
            default:                
               return value.asString();
         }
      }


      DisplayColumn() = default;
      DisplayColumn(const DisplayColumn&) = default;
      DisplayColumn(DisplayColumn&&) = default;
      DisplayColumn& operator=(const DisplayColumn&) = default;
      DisplayColumn& operator=(DisplayColumn&&) = default;
      ~DisplayColumn() = default;
   };


} // namespace ctb