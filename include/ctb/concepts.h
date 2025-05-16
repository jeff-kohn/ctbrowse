/*********************************************************************
 * @file       concepts.h
 *
 * @brief      defines some app-specific concepts
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include <concepts>
#include <ranges>
#include <string>
#include <string_view>

namespace ctb
{
   namespace rng = std::ranges;
   namespace vws = rng::views;

   /// @brief Concept for a type that is convertible to std::string_view
   template <typename T>
   concept StringViewCompatibleType = std::convertible_to<T, std::string_view>;


   /// @brief Concept for a type that can be used as a table property in a table record.
   ///
   /// concept is modeled after interface of ctb::TableProperty template, but any type 
   /// with compatible interface could be used.
   /// 
   template <typename T>
   concept TablePropertyType = std::is_default_constructible_v<T> 
                               and std::is_move_constructible_v<T>
                               and requires (T t, typename T::ValueType v)
   {
      t = T{ v };
      t.setNull();

      { t.isNull()         } -> std::same_as<bool>;
      { t.hasString()      } -> std::same_as<bool>;
      { t.asString()       } -> std::same_as<std::string>;
      { t.asStringView()   } -> std::same_as<std::string_view>;

      { t.asInt32().value_or(0)    } -> std::same_as<int32_t>;
      { t.asUInt16().value_or(0)   } -> std::same_as<uint16_t>;
      { t.asUInt64().value_or(0)   } -> std::same_as<uint64_t>;
      { t.asDouble().value_or(0.0) } -> std::same_as<double>;
   };


   template <typename T>
   concept PropertyMapType = std::is_enum_v<typename T::key_type> 
                             and TablePropertyType<typename T::mapped_type>
                             and requires (T t, typename T::key_type key)
   {
      { t.begin()       } -> std::same_as<typename T::iterator>;
      { t.end()         } -> std::same_as<typename T::iterator>;
      { t.find(key)     } -> std::same_as<typename T::iterator>;
      { t.contains(key) } -> std::same_as<bool>;
      { t[key]          } -> std::assignable_from<typename T::mapped_type>;
   };


   /// @brief Concept for a traits type defining the schema for a TableRecordType instantiation
   ///
   template <typename T> 
   concept RecordTraitsType = requires (typename T::Prop pid, typename T::PropertyMap props)
   {
      { T::Schema.find(pid)->second } -> std::same_as<const typename T::FieldSchema&>;
      { T::DefaultListColumns[0]    } -> std::same_as<const typename T::ListColumn&>;
      { T::AvailableSorts[0]        } -> std::same_as<const typename T::TableSort&>;
      { T::MultiMatchFilters[0]     } -> std::same_as<const typename T::MultiMatchFilter&>;
      { T::getTableName()           } -> std::same_as<std::string_view>;
      { T::hasProperty(pid)         } -> std::same_as<bool>;

      T::getTableId();
      T::onRecordParse(props);
   };


   /// @brief Concept for a record object representing a row in a CT table (CSV file)
   ///
   template <typename T> 
   concept TableRecordType = requires (T t, 
      typename T::Prop pid, 
      typename T::Property prop, 
      typename T::RowType row, 
      std::string_view sv)
   {
     t.parseRow(row);
     t.hasProperty(pid);
     t.getProperty(T::Prop::iWineId);
     prop = t[pid];
     t.getProperties();
   };


   template <typename T>
   concept DataTableType = rng::random_access_range<T> and requires (T t, typename T::value_type::Prop pid)
   {
      { t.size()   } -> std::same_as<size_t>;
      { t[0]       } -> std::same_as<typename T::value_type&>;
      { t[0][pid]  } -> std::same_as<const typename T::value_type::Property&>; 
   };


   /// @brief Concept for a type that implements the interface of std::optional
   ///
   template<typename T> 
   concept NullableType = requires (T t, T::value_type v1, T::value_type v2)
   {
      v1 = t ? *t : v2;
      v1 = t.has_value() ? t.value() : v2;
      v1 = t.value_or(v2);
   };


   /// @brief Concept for a type that is either integral or floating point.
   ///
   template <typename T>
   concept ArithmeticType = std::integral<T> or std::floating_point<T>; 


   /// @brief Concept for an enum type. 
   ///
   template <typename T>
   concept EnumType = std::is_enum_v<T>;


} // namespace ctb
