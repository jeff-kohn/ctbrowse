/*******************************************************************
* @file  CtRecordImpl.h
*
* @brief defines the template class CtRecordImpl as well as some
*        small related types.
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/functors.h"
#include "ctb/TableProperty.h"

#include <external/csv.hpp>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_utility.hpp>

#include <array>
#include <cassert>
#include <optional>
#include <vector>


namespace ctb
{
   /// @brief enum to specify the data formats a property value can contain.
   /// 
   /// these enums indicate how to attempt to parse/interpret the CSV field for a given property.
   ///
   enum PropType
   {
      String,
      UInt16,
      UInt64,
      Double
   };


   /// @brief  contains the property type and CSV column index for a given PropId
   ///
   struct FieldSchema
   {
      uint32_t prop_idx{};
      PropType prop_type{};
      uint32_t csv_col_idx{};
   };


   /// @brief base class providing common functionality for our table record objects
   /// 
   /// this class provides support for parsing CSV records into C++ objects. The derived class
   /// must meet the requirements of the CtRecordTraits concept.
   /// 
   /// it may seem a little odd to use a template param and deducing this for CRTP, but the 
   /// template param gives us access to types from the derived class from outside function
   /// bodies (e.g. member variables).
   /// 
   template<CtRecordTraits RecordTraits>
   class CtRecordImpl
   {
   public:
      using Traits        = RecordTraits;
      using PropId        = RecordTraits::PropId;
      using TableProperty = TableProperty<uint16_t, uint64_t, double, std::string>;
      using Properties    = std::array<TableProperty, magic_enum::enum_count<PropId>()>;
      using RowType       = csv::CSVRow;

      /// @brief parse a CSVRow into TableProperties for each property in m_props
      ///
      void parse(const RowType& row)
      {
         using namespace magic_enum;
         using std::to_underlying;


         // For all the prop_id's in the CSV schema, we get their value from a column
         // in the CSV file. For the remaining, we give the Traits class a chance to
         // provide a calculated value.
         const auto& schema = Traits::getCsvSchema();
         auto props = magic_enum::enum_values<PropId>();
         for (auto prop_id : props)
         {
            auto it = schema.find(prop_id);
            if (it != schema.end())
            {
               auto& fld = it->second;
               assert(fld.prop_idx < std::ssize(row));

               // csv:: objects throw on error
               try
               {
                  auto csv_field = row[fld.csv_col_idx];
                  m_props[fld.prop_idx] = parse_field(csv_field, fld.prop_type);
               }
               catch(...)
               {
                  // not much else we can do
                  assert("parsing field from CSV encountered unexpected error");
                  m_props[fld.prop_idx].setNull();
               }
            }
            else{
               // give the traits class a chance to provide the value.
               Traits::getCalculatedValue(std::span(m_props), prop_id);
            }
         }
      }


      /// @brief get the property corresponding to the specified enum identifier
      ///
      [[nodiscard]] const TableProperty& getProperty(PropId prop) const
      {
         return getProperty(std::to_underlying(prop));
      }


      /// @brief get the property based on enum index
      ///
      [[nodiscard]] const TableProperty& getProperty(int col_idx) const
      {
         // throwing from here would not be useful, because if we get an incorrect index it's a bug 
         // of some sort and will probably generate an exception for each row in the table, which makes
         // displaying an error message to the user a bad idea.
         if (col_idx >= std::ssize(m_props))
            assert("this is a bug, property index out of range");

         return m_props[static_cast<size_t>(col_idx)];
      }


      /// @brief array syntax for getting a property value
      ///
      [[nodiscard]] const TableProperty& operator[](PropId prop) const 
      {
         return getProperty(std::to_underlying(prop));
      }


      /// @brief array syntax for getting a property value
      ///
      [[nodiscard]] const TableProperty&& operator[](int col_idx) const 
      {
         return getProperty(col_idx);
      }


   private:
      // array to hold a property value for every entry in the PropId enum. 
      //
      Properties m_props{};

      // @brief converts a CSVField into a TableProperty
      //
      TableProperty parse_field(csv::CSVField& fld, PropType prop_type)
      {
         if (fld.is_null())
            return {};

         switch (prop_type)
         {
            case PropType::String:
               return TableProperty{ fld.get<std::string>() };

            case PropType::UInt16:
               return fld.is_int() ? TableProperty{ fld.get<uint16_t>() } : TableProperty{};

            case PropType::UInt64:
               return fld.is_int() ? TableProperty{ fld.get<uint16_t>() } : TableProperty{};

            case PropType::Double:
            {
               long double val{};
               if (fld.try_parse_decimal(val))
                  return TableProperty{ static_cast<double>(val) };
               else
                  return {};
            }
            default:
               assert(false);
               throw Error{ "PropType enum contains unexpected value, this is a bug" };
         }

      }
   };





} // namespace ctb