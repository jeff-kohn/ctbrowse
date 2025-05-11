/*******************************************************************
 * @file MultiMatchPropertyFilter.h
 *
 * @brief defines the template class MultiMatchPropertyFilter
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"


namespace ctb::detail
{

   /// @brief template class that implements filter logic that selects records if a property matches
   /// any one of a set of match values.
   /// 
   /// Note that no type-coercion is used when comparing variants. The match_values should have the 
   /// same variant type as the table property that is being filtered.
   /// 
   template<EnumType PropT, PropertyMapType PropMapT>
   struct MultiMatchPropertyFilter
   {
      using Prop          = PropT;
      using PropertyMap    = PropMapT;
      using Property = PropertyMap::mapped_type;
      using MatchValues   = std::set<Property>;

      /// @brief Property that we're filtering against
      Prop prop_id{};

      /// @brief User-facing name
      std::string filter_name{};

      /// @brief Possible values to match against
      MatchValues match_values{};

      /// @brief returns true if the specified record is a match or we have no match values to check.
      bool operator()(const PropertyMap& rec) const noexcept
      {
         if (match_values.empty())
            return true;

         auto it = rec.find(prop_id);
         if (it == rec.end() or it->second.isNull())
            return false;

        return match_values.find(it->second) != match_values.end();
      }
   };


} // namespace ctb::detail
