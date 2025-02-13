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


   /// @brief retrieve a list of possible filter values for the given property in the table
   ///
   template<rng::input_range Rows, typename PropEnum> 
   std::set<std::string> getFilterMatchValues(const Rows& rows, PropEnum prop_id)
   {
      // this functor turns our field values into strings
      // Note, we need both the string and string_view overloads 
      auto FieldToStr = Overloaded {
         [](const std::string& val)      { return std::format("{}", val); },
         [](const std::string_view val)  { return std::format("{}", val); },
         [](const auto& val)             { return std::format("{}", val); },
         [](const NullableShort val)     { return val.has_value() ? std::format("{}", val.value()) : std::string{}; },
         [](const NullableDouble val)    { return val.has_value() ? std::format("{}", val.value()) : std::string{}; }
      };

      /// vws::transform() is considered mutating and cannot be called on a const range, because the 
      /// whole ranges/views library is fucked when it comes to const-correctness and is a real step back for the 
      /// language, so now I have to write a motherfucking for loop. I supposed I could use ranges::for_each
      /// but it has no advantages and is uglier.
      /// 
      std::set<std::string> result{};
      for (auto& row : rows)
      {
         auto val_result = row[prop_id];
         if (val_result)
         {
            auto val = std::visit(FieldToStr, *val_result);
            result.insert(val);
         }
      }
      return result;
   }

} // namespace ctb::data
