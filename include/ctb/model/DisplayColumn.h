/*******************************************************************
 * @file DisplayColumn.h
 *
 * @brief defines the template class DisplayColumn
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once


#include <ctb/tables/TableProperty.h>

#include <vector>

namespace ctb::app
{

   /// @brief struct containing everything needed to know about how to display a table column
   ///
   struct DisplayColumn
   {
      /// @brief enum to specify the alignment for column headers and cell text
      ///
      /// these values align with wxWidgets' wxALIGN_xxxx values, but we don't want the dependency
      /// in the lib so just use the value directly.
      /// 
      enum Align : uint16_t
      {
         Left     = 0x0000,
         Right    = 0x0200,
         Center   = 0x0900
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

      /// @brief The zero-based index into the record type's PropId enum of the property this object represents 
      ///
      /// We have to use a int instead of the enum because this class needs to be used through a type-erased 
      /// interface and can't refer to types specific to a TableRecord<> instantiation.
      /// 
      int prop_index{};                  

      /// @brief Title to use for the column's header 
      std::string display_name{};

      /// @brief The format to use when displaying the value 
      Format format{ Format::String };    
      
      /// @brief How the column's values should be aligned
      Align col_align{ Align::Left };    

      /// @brief How the column header should be aligned
      Align header_align{ Align::Left }; 

      /// @brief construct a column to display the specified property as a string
      ///
      /// column header value is option and will use the table column name by default
      ///
      explicit DisplayColumn(int prop_idx, std::string_view col_name) : 
         prop_index{ prop_idx }, 
         display_name{ col_name }
      {}

      /// @brief construct a column to display the specified property in the requested format
      ///
      /// column header value is optional and will use the table column name by default
      ///
      DisplayColumn(int prop_idx, Format fmt, std::string_view col_name) : 
         prop_index{ prop_idx }, 
         display_name{ col_name }, format{ fmt }
      {
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
      template<typename... Args>
      std::string getDisplayValue(const TableProperty<Args...>& value) const
      {       
         switch (format)
         {
            case Format::Decimal:   
               return value.asString(constants::FMT_NUMBER_DECIMAL);

            case Format::Currency:  
               return value.asString(constants::FMT_NUMBER_CURRENCY);

            case Format::Number: // regular numbers don't have special formatting except for being right-aligned.   
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

   using DisplayColumns = std::vector<DisplayColumn>;

} // namespace ctb