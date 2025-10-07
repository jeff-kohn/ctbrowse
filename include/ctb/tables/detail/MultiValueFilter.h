/*******************************************************************
 * @file MultiValueFilter.h
 *
 * @brief defines the template class MultiValueFilter
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
   struct MultiValueFilter
   {
      using Prop         = PropT;
      using PropertyMap  = PropMapT;
      using PropertyVal  = PropertyMap::mapped_type;
      using MatchValues  = std::set<PropertyVal>;

      /// @brief Property that we're filtering against
      Prop prop_id{};

      /// @brief User-facing name
      std::string filter_name{};

      /// @brief If true, match_values should be displayed to user in descending order. This is a suggestion,
      /// but it doesn't affect the actual sort order of match_values, caller will have to reverse it.
      bool reverse_match_values{ false };

      /// @brief Possible values to match against. This is a default-sorted set
      MatchValues match_values{};

      /// @brief When enabled==false, operator() will always return true
      bool enabled{ true };

      /// @brief returns true if the specified record is a match or we have no match values to check.
      auto operator()(const PropertyMap& rec) const noexcept -> bool
      {
         if (match_values.empty() or !enabled)
            return true;

         auto it = rec.find(prop_id);
         if (it == rec.end())
            return false;

        return match_values.find(it->second) != match_values.end();
      }
   };


} // namespace ctb::detail
