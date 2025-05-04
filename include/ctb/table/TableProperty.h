/*******************************************************************
* @file  TableProperty.h
*
* @brief defines the template class TableProperty
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/utility_templates.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>



namespace ctb
{

   /// @brief class to contain a property value from a table entry.
   ///
   /// this class is a lightweight wrapper around std::variant that is easier use and provides a built-in
   /// concept of 'null' so that we don't have to handle a mix of normal and std::optional<> types.
   /// 
   /// We use std::monostate to indicate null, but callers don't need to bother with that, just use
   /// setNull()/isNull() before trying to get a non-string value out of this object. This class doesn't
   /// differentiate between Null and "" for string properties, because there's no way to distinguish them
   /// as distinct values in CSV. Default-constructed TableProperties will be null until assigned a non-null value.
   /// 
   /// Note that while it's possible to put std::optional<> types into this class, there's not much 
   /// point since it has built-in support for null/empty in the variant.
   /// 
   /// Default-constructed instances of this object will always have a 'null' value.
   /// 
   template<typename... Args>
   struct TableProperty
   {
      using ValueType = std::variant<std::monostate, Args...>;

      /// @brief construct a TableProperty from any value convertible to ValueType
      ///
      /// this is intentionally non-explicit, because callers won't always have the exact
      /// typename easily available.
      template<std::convertible_to<ValueType> T>
      constexpr TableProperty(T&& val) : m_val{ std::forward<T>(val) }
      {}

      /// @brief returns whether or not this object contains a 'null' value.
      ///
      bool isNull() const
      {
         return m_val.index() == 0;
      }

      /// @brief sets the contained value of this object to represent 'null' (e.g. std::monostate)
      /// 
      void setNull()
      {
         m_val = std::monostate{};
      }

      /// @brief get a numeric value out of the property
      /// 
      /// returns an optional in case this property doesn't contain a value or it can't be converted to T.
      /// 
      template<Arithmetic T> 
      constexpr std::optional<T> as() const
      {    
         // try to convert
         auto asT = Overloaded
         {
            [](const std::monostate) -> std::optional<T> { return std::nullopt; },
            [](const std::string& str) -> std::optional<T> { return from_str<T>(str); },
            [](auto val) -> std::optional<T> { return std::optional<T>(static_cast<T>(val)); }
         };
         return std::visit(asT, m_val);
      }

      /// @brief get a string value out of the property
      /// 
      /// @return the requested value, or an empty string if no value is available (e.g. null) 
      /// 
      std::string asString() const
      {
         if (std::holds_alternative<std::string>(m_val))
         {
            return std::get<std::string>(m_val);
         }

         return asString("{}");
      }

      /// @brief return string_view to the internal string property
      /// @return the requested string_view, or an empty one if this property doesn't contain a string.
      /// 
      /// this method does not convert other types to string_view, because that would require a view on a temporary. 
      /// if the contained property is not a valid string, you'll get an empty string_view back.
      std::string_view asStringView() const
      {
         if (std::holds_alternative<std::string>(m_val))
         {
            return std::get<std::string>(m_val);
         }
         return {};
      }

      /// @brief indicates whether this property contains a std::string
      /// @return true if the property type is std::string, false if it's anything else
      /// 
      /// this can be useful in determining whether you want to call asString() or asStringView()
      /// since the former will convert numbers to string and the latter will not.
      /// 
      constexpr bool hasString() const
      {
         return std::holds_alternative<std::string>(m_val);
      }

      /// @brief convenience function getting value as int32_t
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      std::optional<int32_t> asInt32() const
      {
         return as<int32_t>();
      }

      /// @brief convenience function getting value as uint16_t
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      std::optional<uint16_t> asUInt16() const
      {
         return as<uint16_t>();
      }

      /// @brief convenience function getting value as uint64_t
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      std::optional<uint64_t> asUInt64() const
      {
         return as<uint64_t>();
      }

      /// @brief convenience function getting value as double
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      std::optional<double> asDouble() const
      {
         return as<double>();
      }

      /// @brief get a formatted string value out of the property.
      /// @param fmt_str format string to use for formatting the value. must contain exactly 1 {} placeholder
      /// @return the requested value, or an empty string if isNull(). 
      /// 
      /// Note that if the property isNull(), the fmt_str will not be used - you will always get an empty string
      /// 
      std::string asString(std::string_view fmt_str) const
      {
         auto asStr = Overloaded
         {
            [](const std::string& val) {  return val;                                                 },
            [](std::monostate) {  return std::string{};                                       },
            [&fmt_str](auto val) {  return ctb::vformat(fmt_str,  ctb::make_format_args(val));  }
         };
         return std::visit(asStr, m_val);
      }

      /// @brief allows for comparison of TableProperty objects, as well as putting them in ordered containers
      /// 
      [[nodiscard]] auto operator<=>(const TableProperty& prop) const 
      {
         return m_val <=> prop.m_val;
      }
            
      /// @brief allow assigning values, not just TableProperties
      ///
      template<typename Self, std::convertible_to<ValueType> T>
      auto&& operator=(this Self&& self, T&& t) 
      {
         self.m_val = std::forward<T>(t);         
         return std::forward<Self>(self);
      }

      /// @brief allow checking for null in a conditional statement
      /// @return true if this object contains a non-null value, false if its value is null
      operator bool() const
      {
         return !isNull();
      }

      constexpr TableProperty() noexcept = default;
      ~TableProperty() noexcept = default;
      constexpr TableProperty(const TableProperty&) = default;
      constexpr TableProperty(TableProperty&&) = default;
      constexpr TableProperty& operator=(const TableProperty&) = default;
      constexpr TableProperty& operator=(TableProperty&&) = default;
   
   private:
      ValueType m_val{};
   };


}  // namespace ctb