/*******************************************************************
 * @file ColumnFilter.h
 *
 * @brief defines the template class ColumnFilter
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/data/table_data.h"
#include "ctb/functors.h"

#include <string>
#include <set>
#include <variant>

namespace ctb::data
{

   /// @brief class that can be used to filter table entry records based
   ///        on one or more values for a given property.
   /// 
   template <TableEntry TblEntry>
   struct ColumnFilter
   {
      // some types we borrow from our template parameter
      using Prop         = TblEntry::Prop;
      using ValueWrapper = TblEntry::ValueWrapper;

      /// @brief the property that we're filtering against
      ///
      Prop prop_id{};

      /// @brief the possible values to match against
      ///
      std::set<ValueWrapper> m_filter_values{};

      /// @brief returns true if the specified table entry is a match
      ///
      bool operator()(TblEntry te)
      {
         auto maybe_prop_val = te[prop_id];
         if (maybe_prop_val)
         {
            return m_filter_values.find(*maybe_prop_val) != m_filter_values.end();
         }
         return false;
      }
   };

   template<rng::input_range Rows, TableEntry TblEntry> requires std::is_same_v<rng::range_value_t<Rows>, TblEntry>
   auto getFilterValues(Rows&& rows, typename TblEntry::Prop prop_id)
   {
      // this functor turns our field values into strings
      // Note, we need both the string and string_view overloads 
      auto FieldToStr = Overloaded{
         [](const std::string& val) { return std::format("{}", val); },
         [](std::string_view val)   { return std::format("{}", val); },
         [](auto&& val)             { return std::format("{}", val); },
         [](NullableShort val)      { return val.has_value() ? std::format("{}", val.value()) : ""; },
         [this](NullableDouble val) { return val.has_value() ? std::format("{}", val.value()) : ""; }
      };

      auto result = rows | vws::transform([prop_id](TblEntry& row)
                              {
                                 auto maybe_prop = row[prop_id].or_else([](auto) -> TblEntry::ValueResult 
                                    { 
                                       return { typename TblEntry::ValueWrapper{} }; 
                                    });
                                 return std::visit(FieldToStr, maybe_prop);
                              })
                         | rng::to<std::set>(); // automatic de-dup

   }

} // namespace ctb::data
