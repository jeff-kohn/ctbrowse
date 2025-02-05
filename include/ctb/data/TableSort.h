/*******************************************************************
 * @file table_sort.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/functors.h"

#include <compare>
#include <vector>

namespace ctb::data
{
   template<TableEntry T>
   struct TableSort
   {
      using Prop = T::Prop;

      std::vector<Prop> sort_props{};   // properties to use for soring, in order
      std::string       sort_name{};    // for display purposes in selection lists etc

      /// @brief function operator that does the comparison.
      bool operator()(const T& t1, const T& t2)
      {
         for (auto prop : sort_props)
         {
            // If returned property doesn't contain the expected ValueResult, just use a default-constructed one.
            const auto& val1 = t1[prop].or_else([](auto) -> T::ValueResult { return { typename T::ValueWrapper{} }; });
            const auto& val2 = t2[prop].or_else([](auto) -> T::ValueResult { return { typename T::ValueWrapper{} }; });

            auto cmp = *val1 <=> *val2;
            if (cmp < 0)
               return true;
            else if (cmp > 0)
               return false;
         }
         return false; // all props equal
      }
   };

}  // ctb::data