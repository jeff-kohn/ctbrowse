/*******************************************************************
* @file  TableRecord.h
*
* @brief defines the template class TableRecord as well as some
*        small related types.
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/CtProperty.h"

#include <boost/unordered/unordered_flat_map.hpp>
#include <external/csv.hpp>
#include <magic_enum/magic_enum.hpp>

#include <cassert>


namespace ctb
{
   /// @brief enum to specify the data formats a property value can contain.
   /// 
   /// these enums indicate how to attempt to parse/interpret the CSV field for a given property.
   ///
   enum class PropType
   {
      String,
      UInt16,
      UInt64,
      Double,
      Date
   };


   /// @brief  contains the property type and CSV column index for a given Prop
   ///
   struct FieldSchema
   {
      using Prop = CtProp;

      Prop           prop_id{};
      PropType       prop_type{};
      NullableShort  csv_col{};   // will be std::nullopt for custom fields not in the CSV
   };



   /// @brief base class providing common functionality for our table record objects
   /// 
   /// this class provides support for parsing CSV records into C++ objects. The derived class
   /// must meet the requirements of the RecordTraitsType concept.
   /// 
   /// it may seem a little odd to use a template param and deducing this for CRTP, but the 
   /// template param gives us access to types from the derived class from outside function
   /// bodies (e.g. member variables).
   /// 
   template<RecordTraitsType RecordTraitsT, PropertyMapType PropertyMapT>
   class TableRecord
   {
   public:
      using Traits        = RecordTraitsT;
      using Prop          = Traits::Prop;
      using PropertyMap   = PropertyMapT;
      using Property      = PropertyMap::mapped_type; 
      using RowType       = csv::CSVRow;

      /// @brief Construct a TableRecord from a RowType
      explicit TableRecord(const RowType& row)
      {
         parseRow(row);
      }

      /// @brief Construct a TableRecord from a PropertyMap.
      explicit TableRecord(PropertyMap props) : m_props{ std::move(props) }
      {}
      TableRecord(const TableRecord&) = default;
      TableRecord(TableRecord&&) = default;
      TableRecord& operator=(TableRecord&&) = default;

      /// @brief parse a CSVRow into TableProperties for each property in m_props
      ///
      void parseRow(const RowType& row)
      {
         using namespace magic_enum;

         // parse all the CSV properties
         auto csv_cols = vws::values(Traits::getSchema())
                       | vws::filter([](auto& field) { return field.csv_col.has_value(); });

         for (auto& fld_schema : csv_cols)
         {
            try
            {
               auto csv_field = row[fld_schema.csv_col.value()];
               m_props[fld_schema.prop_id] = fieldToProperty(csv_field, fld_schema.prop_type);
            }
            catch (...)
            {
               m_props[fld_schema.prop_id].setNull();

               SPDLOG_DEBUG("TableRecord::Parse() encountered error parsing field {}. {}",
                  enum_name(fld_schema.prop_id),
                  packageError().formattedMesage());
            }
         }

         auto custom_props = vws::values(Traits::getSchema())
                           | vws::filter([](auto& field) { return !field.csv_col.has_value(); });

         // give the traits class a chance to provide any missing values (calculated values not in the CSV)
         Traits::onRecordParse(m_props);
      }

      /// @brief Indicates whether the requested property is available in this record
      /// @return true if the property exists, false if not
      /// 
      auto hasProperty(Prop prop_id) const -> bool
      {
         return m_props.contains(prop_id);
      }

      /// @brief Get the property value corresponding to the property identifier
      /// 
      /// If you need to distinguish between null and missing properties, you should 
      /// check hasProperty() first.
      /// 
      /// @return the requested property value if found, or a null property value if not
      ///
      auto getProperty(Prop prop_id) const -> const Property& 
      {
         auto it = m_props.find(prop_id);
         return it == m_props.end() ? null_prop : it->second;
      }

      /// @brief Get the property value corresponding to the property identifier
      /// 
      /// If you need to distinguish between null and missing properties, you should 
      /// check hasProperty() first.
      /// 
      /// @return the requested property value if found, or a null property value if not
      ///
      auto operator[](Prop prop_id) const -> const Property& 
      {
         return getProperty(prop_id);
      }

      /// @brief  Gets a reference to the map of all properties for this record.
      /// 
      auto getProperties() const -> const PropertyMap&
      {
         return m_props;
      }

      TableRecord() = delete;
      TableRecord& operator=(const TableRecord&) = delete;

   private:
      PropertyMap m_props{ Traits::getSchema().size()};

      // @brief converts a CSVField into a TableProperty
      Property fieldToProperty(csv::CSVField& fld, PropType prop_type)
      {
         if (fld.is_null())
            return {};

         switch (prop_type)
         {
            case PropType::String:
               return Property{ fld.get<std::string>() };

            case PropType::UInt16:
               return fld.is_int() ? Property{ fld.get<uint16_t>() } : Property{};

            case PropType::UInt64:
               return fld.is_int() ? Property{ fld.get<uint16_t>() } : Property{};

            case PropType::Double:
            {
               long double val{};
               if (fld.try_parse_decimal(val))
                  return Property{ static_cast<double>(val) };
               else
                  return {};
            }
            default:
               assert(false);
               throw Error{ "PropType enum contains unexpected value, this is a bug" };
         }

      }
   };


} // namespace ctb::detail