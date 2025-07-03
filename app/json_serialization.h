#pragma once

#include "App.h"

#include <ctb/tables/CtSchema.h>
#include <ctb/utility_chrono.h>

#include <glaze/glaze.hpp>


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


/// @brief json serialization support for CtPropertyVal
//template <>
//struct glz::meta<ctb::CtPropertyVal> {
//   static constexpr auto value = [](auto& self) -> auto& { return self.m_val; };
//};


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
         serialize<JSON>::op<Opts>(ctb::toIsoDate(value), args...);
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

         PropType prop_type{ PropType::String }; // tells us how to parse the json value

         parse<JSON>::op<Opts>(prop_type, ctx, args...);
         if (prop_type == PropType::Date)
         {
            std::chrono::year_month_day ymd{};
            parse<JSON>::op<Opts>(ymd, ctx, args...);
            if (ctx.error == error_code::none)
            {
               value = ymd;
            }
         }
         else {
            std::string value_str{};                
            parse<JSON>::op<Opts>(value_str, ctx, args...);
            switch (prop_type)
            {
               case PropType::Null:
                  value.setNull();
                  break;

               case PropType::String:
                  value = value_str;
                  break;

               case PropType::UInt16:
                  value = CtPropertyVal::parse<uint16_t>(value_str);
                  break;

               case PropType::UInt64:
                  value = CtPropertyVal::parse<uint64_t>(value_str);
                  break;

               case PropType::Double:
                  value = CtPropertyVal::parse<double>(value_str);
                  break;

               default:
                  assert(false);
            }
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
         using namespace ctb;
         using std::chrono::year_month_day;

         auto serializeFunc = ctb::Overloaded
         {
            [&](const std::string& val)     -> void { serialize<JSON>::op<Opts>(std::make_pair(PropType::String, val),          args...);  },
            [&](uint16_t val)               -> void { serialize<JSON>::op<Opts>(std::make_pair(PropType::UInt16, val),          args...);  },
            [&](uint64_t val)               -> void { serialize<JSON>::op<Opts>(std::make_pair(PropType::UInt64, val),          args...);  },
            [&](double val)                 -> void { serialize<JSON>::op<Opts>(std::make_pair(PropType::Double, val),          args...);  },
            [&](const year_month_day& val)  -> void { serialize<JSON>::op<Opts>(std::make_pair(PropType::Date,   val),          args...);  },
            [&](const std::monostate)       -> void { serialize<JSON>::op<Opts>(std::make_pair(PropType::Null,   std::nullopt), args...);  }
         };
         std::visit(serializeFunc, value.variant());
      }
   };

} // namespace glz