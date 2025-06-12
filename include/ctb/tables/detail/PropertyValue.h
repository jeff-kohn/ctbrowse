/*******************************************************************
* @file  PropertyValue.h
*
* @brief defines the template class PropertyValue
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/utility_chrono.h"
#include "ctb/utility_templates.h"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <variant>



namespace ctb::detail
{

   /// @brief class to contain a property value from a table record.
   ///
   /// this class is a lightweight wrapper around std::variant that is easier use and provides a built-in
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
   struct PropertyValue
   {
      using ValueType = std::variant<std::monostate, Args...>;

      /// @brief Create a PropertyValue from a string_view by converting it to the specified type.
      /// 
      /// This is a zero-copy alternative to creating a PropertyValue<string> and then using as<> to convert it.
      /// 
      /// @param text_value - The string_view to be converted and used to construct the PropertyValue.
      /// @return An PropertyValue containing the converted value if the conversion succeeds, or an empty 
      ///  PropertyValue otherwise.
      template<std::convertible_to<ValueType> ValT>
      [[nodiscard]] static auto create(std::string_view text_value) -> PropertyValue
      {
         auto val = from_str<ValT>(text_value); 
         if (val)
         {
            return PropertyValue{ *val };
         }
         else {
            return PropertyValue{};
         }
      }


      /// @brief construct a PropertyValue from any value convertible to ValueType
      ///
      /// this is intentionally non-explicit, because callers won't always have the exact
      /// typename easily available.
      template<std::convertible_to<ValueType> T>
      constexpr PropertyValue(T&& val) : m_val{ std::forward<T>(val) }
      {}

      /// @brief returns whether or not this object contains a 'null' value.
      ///
      auto isNull() const -> bool
      {
         return m_val.index() == 0;
      }

      /// @brief sets the contained value of this object to represent 'null' (e.g. std::monostate)
      /// 
      void setNull()
      {
         m_val = std::monostate{};
      }

      /// @brief Get a numeric value out of the property
      /// 
      /// If the property contains a string, parsing will be attempted. For anything else, the result
      /// will be a static_cast to T if possible, or std::nullopt if not.
      /// 
      /// @returns an optional containing the result if successful, std::nullopt if not.
      /// 
      /// Note: compile error in this function means the type you're casting to isn't compatible with any
      /// of the overloads, either use a different type or add a new overload to asT for the needed type.
      template<ArithmeticType T> 
      constexpr auto as() const -> std::optional<T>
      { 
         using std::chrono::year_month_day;
         using MaybeT = std::optional<T>; 

         // try to convert
         auto asT = Overloaded
         {
            [](const std::monostate)    -> MaybeT { return std::nullopt;                            },
            [](const std::string& str)  -> MaybeT { return from_str<T>(str);                        },
            [](const year_month_day&)   -> MaybeT { return {}; /* no conversion that makes sense */ },
            [](const auto& val)         -> MaybeT { return std::optional<T>(static_cast<T>(val));   }
         };
         return std::visit(asT, m_val);
      }

      /// @brief Extract date value from the property object, if possible
      /// 
      /// The object must contain a year_month_date, or a string that can be parsed as a date. 
      /// All other data types will result in empty return value.
      /// 
      /// @return A valid year_month_date, or std::nullopt if the value is not compatible
      auto asDate() const -> NullableDate
      {
         NullableDate result{};
         if (std::holds_alternative<chrono::year_month_day>(m_val))
         {
            // bypass visit()'s virtual-fn overhead and just directly return.
            result = std::get<chrono::year_month_day>(m_val);
         }
         else if (std::holds_alternative<std::string>(m_val))
         {
            // try to parse
            if (auto parse_result = parseDate(std::get<std::string>(m_val), constants::FMT_PARSE_DATE_SHORT); parse_result.has_value())
            {
               result = *parse_result;
            }
         }
         // for other types nothing makes sense except empty/null
         return result;
      }

      /// @brief get a string value out of the property
      /// 
      /// @return the requested value, or an empty string if no value is available (e.g. null) 
      auto asString() const -> std::string
      {
         if (std::holds_alternative<std::string>(m_val))
         {
            return std::get<std::string>(m_val);
         }

         // For other types we use format() to get a string
         return asString("{}");
      }

      /// @brief get a formatted string value out of the property.
      /// @param fmt_str format string to use for formatting the value. Must contain exactly 1 {} placeholder
      /// @return the requested value, or an empty string if isNull(). 
      /// 
      /// Note that if the property isNull(), the fmt_str will not be used - you will always get an empty string
      /// 
      auto asString(std::string_view fmt_str) const -> std::string 
      {
         using std::chrono::year_month_day;
         using namespace constants;

         if (std::holds_alternative<std::string>(m_val))
         {
            // bypass visit()'s virtual-fn overhead and just directly return.
            auto& str = std::get<std::string>(m_val);
            return ctb::vformat(fmt_str, ctb::make_format_args(str));
         }

         auto asStr = Overloaded
         {
            [](std::monostate)                    {  return std::string{}; },
            [&fmt_str](const year_month_day& val) {  return ctb::vformat(fmt_str == FMT_DEFAULT_FORMAT ? FMT_DATE_SHORT : fmt_str, make_format_args(val)); },
            [&fmt_str](auto val)                  {  return ctb::vformat(fmt_str,  ctb::make_format_args(val)); }
         };
         return std::visit(asStr, m_val);
      }

      /// @brief return string_view to the internal string property
      /// @return the requested string_view, or an empty one if this property doesn't contain a string.
      /// 
      /// this method does not convert other types to string_view, because that would require a view on a temporary. 
      /// if the contained property is not a valid string, you'll get an empty string_view back.
      auto asStringView() const -> std::string_view
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
      constexpr auto hasString() const -> bool
      {
         return std::holds_alternative<std::string>(m_val);
      }

      /// @brief convenience function getting value as int32_t
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      auto asInt32() const -> NullableInt
      {
         return as<int32_t>();
      }

      /// @brief convenience function getting value as uint16_t
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      auto asUInt16() const -> NullableShort
      {
         return as<uint16_t>();
      }

      /// @brief convenience function getting value as uint64_t
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      auto asUInt64() const -> NullableSize_t
      {
         return as<uint64_t>();
      }

      /// @brief convenience function getting value as double
      /// 
      /// just calls as<>(), but the syntax for doing that outside of this class is ugly due to dependent name BS so wrap it here.
      /// 
      auto asDouble() const -> NullableDouble
      {
         return as<double>();
      }

      /// @brief allows for comparison of PropertyValue objects, as well as putting them in ordered containers
      [[nodiscard]] auto operator<=>(const PropertyValue& prop) const 
      {
         return m_val <=> prop.m_val;
      }
      auto operator==(const PropertyValue& prop) const -> bool = default;
            
      /// @brief allow assigning values, not just TableProperties
      template<typename Self, std::convertible_to<ValueType> T>
      auto&& operator=(this Self&& self, T&& t) 
      {
         self.m_val = std::forward<T>(t);         
         return std::forward<Self>(self);
      }

      /// @brief allow checking for null in a conditional statement
      /// @return true if this object contains a non-null value, false if its value is null
      explicit operator bool() const
      {
         return !isNull();
      }

      constexpr PropertyValue() noexcept = default;
      ~PropertyValue() noexcept = default;
      constexpr PropertyValue(const PropertyValue&) = default;
      constexpr PropertyValue(PropertyValue&&) = default;
      constexpr PropertyValue& operator=(const PropertyValue&) = default;
      constexpr PropertyValue& operator=(PropertyValue&&) = default;
   
   private:
      ValueType m_val{};
   };


}  // namespace ctb::detail