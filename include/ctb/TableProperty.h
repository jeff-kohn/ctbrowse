/*******************************************************************
* @file  TableProperty.h
*
* @brief defines the template class TableProperty
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>


namespace ctb
{
   /// @brief class to contain a property value from a table entry.
   ///
   /// this class is lightweight wrapper around std::variant that is easier use and provides a built-in
   /// concept of 'null' so that we don't have to handle a mix of normal and std::optional<> types.
   /// 
   /// We use std::monostate to indicate null, but callers don't need to bother with that, just use
   /// setNull()/isNull() before trying to get a non-string value out of this object. This class doesn't
   /// differentiate between Null and "" for string properties, because there's no way to distinguish them
   /// as distinct values in CSV.
   /// 
   /// Note that while it's possible to put std::optional<> types into this class, there's not much 
   /// point since it has built-in support for null/empty in the variant.
   /// 
   /// Default-constructed instances of this object will always have a 'null' value.
   /// 
   template<typename... Args>
   struct TableProperty
   {
      //using MaybeShort  = std::optional<uint16_t>;
      //using MaybeDouble = std::optional<double>;
      using ValueType   = std::variant<std::monostate, Args...>;


      /// @brief construct a TableProperty from any value convertible to ValueType
      ///
      /// this is intentionally non-explicit, because callers won't always have the exact
      /// typename easily available.
      template<std::convertible_to<ValueType> T>
      constexpr TableProperty(T&& val) : m_val{ std::forward<T>(val) }
      {}


      /// @brief get a numeric value out of the property
      /// 
      /// returns an optional in case this property doesn't contain a value or it can't be converted to T.
      /// Note that this method does NOT attempt to parse string values into arithmetic types. If this
      /// property is a string, you'll get std::nullopt.
      /// 
      //template<Arithmetic T> 
      //std::optional<T> as() const
      //{
      //   // do we have an exact match?
      //   if (std::holds_alternative<T>(m_val))
      //   {
      //      return std::get<T>(m_val);
      //   }        
      //
      //   // if not, try to convert
      //   auto asT = Overloaded{
      //      [](const Nullable auto& val)              { return val ? static_cast<T>(*val) : std::nullopt; },
      //      [](const StringViewCompatible auto&)      { return std::nullopt; },
      //      [](auto val)                              { return static_cast<T>(val); }
      //   };
      //   return std::visit(asT, m_val);
      //}


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

         auto asStr = Overloaded{
            [](Nullable auto val)   {  return val ? std::format("{}", *val) : std::string{};  },
            [](std::monostate)      {  return std::string{};                                  },
            [](auto val)            {  return std::format("{}", val);                         }
         };
         return std::visit(asStr, m_val);
      }


      /// @brief get a formatted string value out of the property.
      /// @param fmt_str format string to use for formatting the value. must contain exactly 1 {} placeholder
      /// @return the requested value, or an empty string if no value is available 
      /// 
      std::string asString(std::string_view fmt_str) const
      {
         auto asStr = Overloaded{
            [](std::string val)              {  return val;                                                 },
            [](std::monostate)               {  return std::string{};                                       },
            [&fmt_str](auto val)             {  return std::vformat(fmt_str,  std::make_format_args(val));  }
         };
         return std::visit(asStr, m_val);
      }

      /// @brief allows for comparison of TableProperty objects, as well as putting them in ordered containers
      /// 
      [[nodiscard]] auto operator<=>(const TableProperty& prop) const 
      {
         return m_val <=> prop.m_val;
      }


      /// @brief allow checking for null in a conditional statement
      /// @return true if this object contains a non-null value, false if its value is null
      operator bool() const
      {
         return !isNull();
      }

      TableProperty() = default;
      ~TableProperty() = default;
      TableProperty(const TableProperty&) = default;
      TableProperty(TableProperty&&) = default;
      TableProperty& operator=(const TableProperty&) = default;
      TableProperty& operator=(TableProperty&&) = default;
   
   private:
      ValueType m_val{};
   };


}  // namespace ctb