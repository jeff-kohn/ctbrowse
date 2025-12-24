/*********************************************************************
 * @file       PropertyFilter.h
 *
 * @brief      declaration for the PropertyFilter class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/PropertyFilterPredicate.h"

#include <algorithm>
#include <functional>
#include <set>
#include <vector>


namespace ctb::detail
{


   /// @brief Template class that implements filter logic based on checking one or more properties against a predicate and value
   ///  
   ///
   ///  Note that there is no type coercion if Property type is a variant. Comparing variants that aren't holding the
   ///  same type will always evaluate to false.
   /// 
   template<EnumType PropT, PropertyMapType PropMapT>
   struct PropertyFilter
   {
      using Prop         = PropT;
      using PropertyMap  = PropMapT;
      using PropertyVal  = PropertyMap::mapped_type;
      using ComparePred  = PropertyFilterPredicate<PropertyVal>;
      using MatchProps   = std::vector<Prop>;
      using MatchValues  = std::set<PropertyVal>;

      /// @brief simplified constructor for a filter with a single property and match value, using the prop_id for filter name and default equality predicate.
      template<std::convertible_to<PropertyVal> T> 
      constexpr PropertyFilter(Prop prop_id, T&& val, ComparePred compare) :
         filter_name{ magic_enum::enum_name(prop_id) },
         prop_ids{ { prop_id } }, 
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(compare) }
      {}

      /// @brief Full constructor, accepts filter_name, prop id's, compare value, and predicate
      template<std::convertible_to<PropertyVal> ValT>  requires std::convertible_to<ValT, PropertyVal>
      constexpr PropertyFilter(std::string_view name, std::initializer_list<Prop> prop_ids, ValT&& val, ComparePred compare) noexcept : 
         filter_name{ name },
         prop_ids{ prop_ids },
         compare_val{ std::forward<ValT>(val) },
         compare_pred{ std::move(compare) }
      {}
      
      /// @brief The filter_name of this filter
      std::string filter_name{};

      /// @brief The match properties the filter checks against
      MatchProps  prop_ids{};

      /// @brief The value that properties will be compared to.
      PropertyVal compare_val{};

      /// @brief The predicate used for matching
      ComparePred compare_pred{};

      /// @brief When enabled==false, operator() will always return true
      bool enabled{ true };

      /// @brief Check if the table record this filter
      auto operator()(const PropertyMap& rec) const -> bool
      {
         if (!enabled)
            return true;

         auto matcher = [&rec, this](Prop prop_id) -> bool
                        {
                           if (auto it = rec.find(prop_id); it != rec.end())
                           {
                              return compare_pred(it->second, compare_val);
                           }
                           return false;
                        };

         return rng::find_if(prop_ids, matcher) != prop_ids.end();
      }

      /// @brief Equality comparison operator
      auto operator==(const PropertyFilter& other) const -> bool
      {
         return filter_name == other.filter_name and 
                prop_ids    == other.prop_ids    and 
                compare_pred(compare_val, other.compare_val) == other.compare_pred(compare_val, other.compare_val);
      }

      PropertyFilter() = default;
      ~PropertyFilter() noexcept = default;
      PropertyFilter(const PropertyFilter&) = default;
      PropertyFilter(PropertyFilter&&) = default;
      PropertyFilter& operator=(const PropertyFilter&) = default;
      PropertyFilter& operator=(PropertyFilter&&) = default;
   };

} // namespace ctb::detail