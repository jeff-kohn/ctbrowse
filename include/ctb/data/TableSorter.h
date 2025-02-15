/*******************************************************************
 * @file TableSorter.h
 *
 * @brief defines the TableSorter template class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/functors.h"
#include "ctb/data/TableProperty.h"

#include <compare>
#include <vector>


namespace ctb::data
{

   /// @brief defines a functor that can be used to sort a container/range of table entries
   ///
   /// note there's no ascending/descending option, because that decision should be made
   /// by the range passed to the sort algorithm, not here.
   /// 
   template<TableEntry T>
   struct TableSorter
   {
      using Prop = T::Prop;
      using PropertyResult = T::PropertyResult;

      std::vector<Prop> sort_props{};      // properties to use for sorting, in order
      std::string       sort_name{};       // for display purposes in selection lists etc

      /// @brief function operator that does the comparison.
      ///
      bool operator()(const T& t1, const T& t2)
      {
         for (auto prop : sort_props)
         {
            // If returned property doesn't contain the expected PropertyResult, just use a default-constructed one.
            const auto& val1 = t1[prop].or_else([](auto) { return PropertyResult{ TableProperty{} }; });
            const auto& val2 = t2[prop].or_else([](auto) { return PropertyResult{ TableProperty{} }; });

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