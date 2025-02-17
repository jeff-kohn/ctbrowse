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
#include "ctb/TableProperty.h"

#include <compare>
#include <vector>


namespace ctb
{

   /// @brief defines a functor that can be used to sort a container/range of table entries
   ///
   /// note there's no ascending/descending option, because that decision should be made
   /// by the range passed to the sort algorithm, not here.
   /// 
   template<CtRecord Record>
   struct TableSorter
   {
      using PropId = Record::PropId;

      std::vector<PropId> sort_props{};      // properties to use for sorting, in order
      std::string         sort_name{};       // for display purposes in selection lists etc

      /// @brief function operator that does the comparison.
      ///
      bool operator()(const Record& r1, const Record& r2)
      {
         for (auto prop : sort_props)
         {
            auto cmp = r1[prop] <=> r2[prop];
            if (cmp < 0)
               return true;
            else if (cmp > 0)
               return false;
         }
         return false; // all props equal
      }
   };

}  // ctb