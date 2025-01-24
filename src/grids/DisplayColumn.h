/*******************************************************************
 * @file DisplayColumn.h
 *
 * @brief Header file for the class DisplayColumn
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/functors.h"
#include "ctb/data/table_data.h"

#include <magic_enum/magic_enum.hpp>
#include <wx/defs.h>


namespace ctb
{
   using magic_enum::enum_name;


   /// @brief concept for a table entry object representing a row in a CT table
   template <typename T> 
   concept TableEntry = requires (
      T t, 
      typename T::Prop p, 
      typename T::ValueResult v,
      typename T::RowType r)
   {
      v = t.getProperty(p);
      v = t[0];
      t.parse(r);
   };


   /// @brief struct containing everything needed to know about how to display a grid column
   template<TableEntry TE>
   struct DisplayColumn
   {
      // some types we borrow from our template parameter
      using Prop         = TE::Prop;
      using ValueWrapper = TE::ValueWrapper;
      using NullableDouble = ctb::data::NullableDouble;
      

      /// @brief enum to specify the alignment for column headers and cell text
      enum Align
      {
         Left = wxAlignment::wxALIGN_LEFT,
         Right = wxAlignment::wxALIGN_RIGHT,
         Center = wxAlignment::wxALIGN_CENTER
      };

      /// @brief enum to specify the format the value will be displayed in
      enum Format
      {
         String,
         Number,
         Decimal,
         Currency
      };


      DisplayColumn() = default;
      DisplayColumn(const DisplayColumn&) = default;
      DisplayColumn(DisplayColumn&&) = default;
      DisplayColumn& operator=(const DisplayColumn&) = default;
      DisplayColumn& operator=(DisplayColumn&&) = default;
      ~DisplayColumn() = default;


      /// @brief construct a column to display the specified property as a string
      ///
      /// column header value is option and will use the table column name by default
      explicit DisplayColumn(Prop prop, std::string_view col_name = {}) : prop_id{ prop }, display_name{ col_name }
      {
         if (display_name.empty())
         {
            display_name = enum_name(prop);
         }
      }


      /// @brief construct a column to display the specified property in the requested format
      ///
      /// column header value is option and will use the table column name by default
      DisplayColumn(Prop prop, Format fmt, std::string_view col_name = {}) : prop_id{ prop }, display_name{ col_name }, format{ fmt }
      {
         if (display_name.empty())
         {
            display_name = enum_name(prop);
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
      std::string getDisplayValue(const ValueWrapper& val) const
      {
         // this functor turns our field values into strings that can be displayed. 
         // Note, we need both the string and string_view overloads 
         auto FieldToStr = Overloaded{
            [this](const std::string& val) { return std::format("{}", val); },
            [this](std::string_view val)   { return std::format("{}", val); },
            [this](uint64_t val)           { return std::format("{}", val); },
            [this](uint16_t val)           { return std::format("{}", val); },
            [this](NullableDouble val)
            { 
               if (val)
               {
                  switch(this->format)
                  {
                     case Format::Currency: return std::format("${:.2f}", *val);
                     case Format::Decimal:  return std::format("{:.1f}", *val);
                     case Format::Number:   return std::format("{:.0f}", *val);
                     default:               return std::format("{}", *val);
                  }
               }
               else{
                  return std::string{};
               }
            }
         };
         // todo: check perf, may want to use manual visit
         return std::visit(FieldToStr, val);
      }


      Prop         prop_id{};                   /// identifier for the property to display
      std::string  display_name{};              /// column header title to use
      Format       format{ Format::String };    /// format to display the value
      Align        col_align{ Align::Left };    /// how to align the column values in the cell
      Align        header_align{ Align::Left }; /// how to align the column title in the header
   };


} // namespace ctb