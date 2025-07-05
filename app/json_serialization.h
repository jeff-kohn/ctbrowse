#pragma once

#include "App.h"

#include <ctb/tables/CtSchema.h>
#include <ctb/utility_chrono.h>

#include <glaze/glaze.hpp>

#include <optional>


/// @brief json serialization support for CtPropFilterPredicate
template <>
struct glz::meta<ctb::CtPropFilterPredicate>
{
   using T = ctb::CtPropFilterPredicate;
   static constexpr auto value = object("m_predicate_type", custom<&T::setPredicateType, &T::predicateType>);
};


/// @brief json serialization support for CtPropertyFilter
template <>
struct glz::meta<ctb::CtPropertyFilter>
{
   using T = ctb::CtPropertyFilter;
   static constexpr auto value = object("filter_name",  &T::filter_name,
                                        "prop_ids",     &T::prop_ids,
                                        "compare_val",  &T::compare_val,
                                        "compare_pred", &T::compare_pred );
};

namespace ctb::detail
{
   struct PropertyValJson
   {
      ctb::PropType prop_type{ PropType::String };
      std::optional<std::string> value{};
   };
}
namespace glz
{
   /// @brief json serialization support for std::chrono::year_month_day
   template <>
   struct from<JSON, std::chrono::year_month_day>
   {
      template <auto Opts>
      static void op(std::chrono::year_month_day& value, is_context auto&& ctx, auto&&... args)
      {
         std::string date_str{};
         parse<JSON>::op<Opts>(date_str, ctx, args...);
         auto ymd_res = ctb::parseDate(date_str);
         if (ymd_res)
            value = ymd_res.value();
         else
            ctx.error = glz::error_code::parse_number_failure;
      }
   };

   /// @brief json serialization support for std::chrono::year_month_day
   template <>
   struct to<JSON, std::chrono::year_month_day>
   {
      template <auto Opts>
      static void op(const std::chrono::year_month_day& value, auto&&... args)
      {
         serialize<JSON>::template op<Opts>(ctb::toIsoDate(value), args...);
      }
   };


   /// @brief json serialization support for CtPropertyVal
   template <>
   struct from<JSON, ctb::CtPropertyVal>
   {
      template <auto Opts>
      static void op(ctb::CtPropertyVal& value, is_context auto&& ctx, auto&&... args)
      {
         using namespace ctb;

         ctb::detail::PropertyValJson json_val{};
         parse<JSON>::template op<Opts>(json_val, ctx, args...);
         if (ctx.error != error_code::none)
         {
            return value.setNull();
         }

         if (not json_val.value.has_value())
            json_val.prop_type = PropType::Null;

         switch (json_val.prop_type)
         {
            case PropType::Null:
               value.setNull();
               break;

            case PropType::String:
               value = std::move(json_val.value.value());
               break;

            case PropType::UInt16:
               value = CtPropertyVal::parse<uint16_t>(json_val.value.value());
               break;

            case PropType::UInt64:
               value = CtPropertyVal::parse<uint64_t>(json_val.value.value());
               break;

            case PropType::Double:
               value = CtPropertyVal::parse<double>(json_val.value.value());
               break;

            case PropType::Date:
               value = CtPropertyVal::parse<chrono::year_month_day>(json_val.value.value());
               break;

            default:
               assert(false);
         }
      }
   };

   /// @brief json serialization support for CtPropertyVal
   template <>
   struct to<JSON, ctb::CtPropertyVal>
   {
      template <auto Opts>
      static void op(const ctb::CtPropertyVal& value, auto&&... args)
      {
         using namespace ctb::detail;
         using std::chrono::year_month_day;

         auto getType = ctb::Overloaded
         {
            [&](const std::string& val)    { return PropType::String; },
            [&](uint16_t val)              { return PropType::UInt16; },
            [&](uint64_t val)              { return PropType::UInt64; },
            [&](double val)                { return PropType::Double; },
            [&](const year_month_day& val) { return PropType::Date;   },
            [&](const std::monostate)      { return PropType::Null;   }
         };
         
         
         auto str_val = value.asString();
         auto json_type = std::visit(getType, value.variant());
         serialize<JSON>::template op<Opts>(PropertyValJson{ json_type, str_val }, args...);
      }
   };

} // namespace glz