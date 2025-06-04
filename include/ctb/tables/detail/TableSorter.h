/*******************************************************************
 * @file TableSorter.h
 *
 * @brief defines the TableSorter template class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <compare>
#include <vector>


namespace ctb::detail
{

   /// @brief Defines a functor that can be used to sort a container/range of table entries
   ///
   /// Default sort is less-than, if reverse=true it will be greater-than
   /// 
   template<EnumType PropT, PropertyMapType PropMapT>
   struct TableSorter
   {
      using PropertyMap = PropMapT;
      using Prop        = PropT;

      std::vector<Prop>   sort_props{};      // properties to use for sorting, in order
      std::string         sort_name{};       // for display purposes in selection lists etc
      bool                reverse{ false };  // set to true to sort in reverse order

      /// @brief function operator that does the comparison.
      auto operator()(const PropertyMap& r1, const PropertyMap& r2) const -> bool
      {
         static constexpr typename PropertyMap::mapped_type null_prop{};

         // the 'reverse' only applies to first sort prop, not subsequent
         bool first = true; 
         for (auto prop : sort_props)
         {
            auto it1 = r1.find(prop);
            const auto& p1 = (it1 == r1.end()) ? null_prop : it1->second;

            auto it2 = r2.find(prop);
            const auto& p2 = (it2 == r2.end()) ? null_prop : it2->second;

            auto cmp = p1 <=> p2;
            if (cmp < 0)
            {
               return reverse and first ? false : true;
            }
            else if (cmp > 0)
            {
               return reverse and first ? true : false;
            }
            first = false;
         }
         return false; // all props equal
      }

      auto operator==(const TableSorter& other) const -> bool
      {
         return reverse == other.reverse and sort_props == other.sort_props and sort_name == other.sort_name;
      }
   };


}  // namespace ctb::detail