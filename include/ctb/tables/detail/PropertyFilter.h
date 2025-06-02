/*********************************************************************
 * @file       PropertyFilter.h
 *
 * @brief      declaration for the PropertyFilter class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <algorithm>
#include <functional>
#include <vector>


namespace ctb::detail
{

   /// @brief Template class that implements filter logic that selects records if one or more specified
   ///  properties match a given value using the provided predicate (which could be std:less<>, etc)
   ///
   ///  Note that there is no type coercion if Property type is a variant. Comparing variants that aren't holding the
   ///  same type will always evaluate to false.
   template<EnumType PropT, PropertyMapType PropMapT>
   struct PropertyFilter
   {
      using Prop         = PropT;
      using PropertyMap  = PropMapT;
      using Property     = PropertyMap::mapped_type;
      using ComparePred  = std::function<bool(const Property&, const Property&)>;

      /// @brief construct a PropertyFilter from any value convertible to ValueT for the specified property
      template<std::convertible_to<Property> T>
      constexpr PropertyFilter(Prop prop, ComparePred pred, T&& val) noexcept :
         match_props{ prop }, 
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(pred) },
         enabled{ true }
      {}

      /// @brief construct a PropertyFilter for matching the given value and predicate to one of multiple properties       
      template<std::convertible_to<Property> T> 
      constexpr PropertyFilter(std::initializer_list<Prop> props, ComparePred pred, T&& val) noexcept : 
         match_props{ props.begin(), props.end() },
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(pred) },
         enabled{ true }
      {}


      template<std::convertible_to<Property> T> 
      constexpr PropertyFilter(std::string_view name, Prop prop, ComparePred pred, T&& val, bool enabled = true) noexcept : 
         match_props{ prop },
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(pred) },
         filter_name{ name },
         enabled{ enabled }
      {}

      template<std::convertible_to<Property> T> 
      constexpr PropertyFilter(std::string_view name, std::initializer_list<Prop> props, ComparePred pred, T&& val, bool enabled = true) noexcept : 
         match_props{ props.begin(), props.end() },
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(pred) },
         filter_name{ name },
         enabled{ enabled }
      {}

      /// @brief the properties that we're filtering against
      std::vector<Prop> match_props{};

      /// @brief the value the record property will be compared to using compare_pred
      Property compare_val{};

      /// @brief predicate that will be used to compare record properties to compare_val
      ComparePred compare_pred{ std::greater<Property>{} };
      
      /// @brief name of the filter
      std::string filter_name{};

      /// @brief whether this filter is active. If set to false, the operator() will always return true
      bool enabled{ false };

      /// @brief returns true if the specified record is a match 
      auto operator()(const PropertyMap& rec) const -> bool
      {
         auto matcher = [&rec, this](Prop prop_id) -> bool
                        {
                           if (auto it = rec.find(prop_id); it != rec.end())
                           {
                              return compare_pred(it->second, compare_val);
                           }
                           return false;
                        };
         if (enabled)
         {
            return rng::find_if(match_props, matcher) != match_props.end();
         }
         return true;
      }

      auto operator==(const PropertyFilter& other) const -> bool
      {
         return match_props == other.match_props and 
                filter_name == other.filter_name and
                enabled     == other.enabled     and
                compare_pred(compare_val, other.compare_val) == other.compare_pred(compare_val, other.compare_val);
      }

      PropertyFilter() noexcept = default;
      ~PropertyFilter() noexcept = default;
      PropertyFilter(const PropertyFilter&) = default;
      PropertyFilter(PropertyFilter&&) = default;
      PropertyFilter& operator=(const PropertyFilter&) = default;
      PropertyFilter& operator=(PropertyFilter&&) = default;
   };

} // namespace ctb::detail