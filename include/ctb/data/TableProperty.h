#pragma once

#include "ctb/ctb.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>


namespace ctb::data
{
   /// @brief class to contain a property value from a table entry.
   ///
   /// this class is easier and safer to use than a variant.
   /// 
   struct TableProperty
   {
      using MaybeShort  = std::optional<uint16_t>;
      using MaybeDouble = std::optional<double>;
      using ValueType   = std::variant<uint16_t, uint64_t, MaybeShort, MaybeDouble, std::string_view>;


      /// @brief construct a TableProperty from any value convertible to ValueType
      ///
      template<std::convertible_to<ValueType> T>
      explicit constexpr TableProperty(T&& val) : m_val{ std::forward<T>(val) }
      {}


      /// @brief get a numeric value out of the property
      /// 
      /// returns an optional in case this property doesn't contain a value or it can't be converted to T.
      /// Note that this method does NOT attempt to parse string values into arithmetic types. If this
      /// property is a string, you'll get std::nullopt.
      /// 
      template<Arithmetic T> 
      std::optional<T> as() const
      {
         // do we have an exact match?
         if (std::holds_alternative<T>(m_val))
         {
            return std::get<T>(m_val);
         }        

         // if not, try to convert
         auto asT = Overloaded{
            [](const Nullable auto& val)              { return val ? static_cast<T>(*val) : std::nullopt; },
            [](const StringViewCompatible auto&)      { return std::nullopt; },
            [](auto val)                              { return static_cast<T>(val); }
         };
         return std::visit(asT, m_val);
      }


      /// @brief get a string value out of the property
      /// 
      /// @return the requested value, or an empty string if no value is available 
      ///         (or convertible to string).
      /// 
      std::string asString() const
      {
         if (std::holds_alternative<std::string_view>(m_val))
         {
            return std::string{ std::get<std::string_view>(m_val) };
         }

         auto asStr = Overloaded{
            [](Nullable auto val)   {  return val ? std::format("{}", *val) : std::string{};  },
            [](auto val)            {  return std::format("{}", val);                        }
         };
         return std::visit(asStr, m_val);
      }


      /// @brief get a formatted string value out of the property.
      /// @param fmt_str format string to use for formatting the value. must contain exactly 1 {}
      /// @return the requested value, or an empty string if no value is available 
      /// 
      std::string asString(std::string_view fmt_str) const
      {
         auto asStr = Overloaded{
            [](std::string_view val)         {  return std::string{ val };                                },
            [&fmt_str](Nullable auto val)    {  return val ? std::vformat(fmt_str,  std::make_format_args(*val)) : std::string{};  },
            [&fmt_str](auto val)             {  return std::vformat(fmt_str,  std::make_format_args(val));                         }
         };
         return std::visit(asStr, m_val);
      }

      [[nodiscard]] auto operator<=>(const TableProperty& prop) const 
      {
         return m_val <=> prop.m_val;
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


}  // namespace ctb::data