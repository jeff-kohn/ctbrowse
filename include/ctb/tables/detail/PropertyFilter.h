/*********************************************************************
 * @file       PropertyFilter.h
 *
 * @brief      declaration for the PropertyFilter class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/ctb.h"
#include <magic_enum/magic_enum.hpp>

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
   class PropertyFilter
   {
   public:
      using Prop         = PropT;
      using PropertyMap  = PropMapT;
      using PropertyVal  = PropertyMap::mapped_type;
      using ComparePred  = std::function<bool(const PropertyVal&, const PropertyVal&)>;
      using MatchProps   = std::vector<Prop>;
      using MatchValues  = std::set<PropertyVal>;

      /// @brief simplified constructor for a filter with a single property and match value, using the prop_id for filter name and default equality predicate.
      template<std::convertible_to<PropertyVal> T> 
      constexpr PropertyFilter(Prop prop_id, T&& val, ComparePred compare = std::equal_to<PropertyVal>{}) :
         m_filter_name{ magic_enum::enum_name(prop_id) },
         m_prop_ids{ { prop_id } }, 
         m_compare_val{ std::forward<T>(val) },
         m_compare_pred{ std::move(compare) }
      {}

      /// @brief Full constructor, accepts name, propd id's, compare value, and predicate
      template<std::convertible_to<PropertyVal> ValT>  requires std::convertible_to<ValT, PropertyVal>
      constexpr PropertyFilter(std::string_view name, std::initializer_list<Prop> prop_ids, ValT&& val, ComparePred compare = std::equal_to<PropertyVal>{}) noexcept : 
         m_filter_name{ name },
         m_prop_ids{ prop_ids },
         m_compare_val{ std::forward<ValT>(val) },
         m_compare_pred{ std::move(compare) }
      {}
      

      /// @brief Get the name of this filter
      ///
      /// May be a specified name or auto-generated from prop_id depending on initialization
      auto name() const -> const std::string&
      {
         return m_filter_name;
      }

      /// @brief Set the name of this filter
      /// @return self-ref for builder-style interface
      auto setName(std::string name) -> PropertyFilter&
      {
         m_filter_name.swap(name);
         return *this;
      }

      /// @brief The match properties the filter checks against
      template<typename Self>
      auto&& matchProps(this Self&& self)  
      {
         return std::forward<Self>(self).m_prop_ids;
      }

      /// @brief The value that properties will be compared to.
      template<typename Self>
      auto&& matchValue(this Self&& self) 
      {
         return std::forward<Self>(self).m_compare_val;
      }

      /// @brief comparePred
      /// @return - reference to the predicate used for matching
      template<typename Self>
      auto&& comparePred(this Self&& self)
      {
         return std::forward<Self>(self).m_compare_pred;
      }

      /// @brief Check if the table record contains one of our match falues.
      auto operator()(const PropertyMap& rec) const -> bool
      {
         auto matcher = [&rec, this](Prop prop_id) -> bool
                        {
                           if (auto it = rec.find(prop_id); it != rec.end())
                           {
                              return m_compare_pred(it->second, m_compare_val);
                           }
                           return false;
                        };

         return rng::find_if(m_prop_ids, matcher) != m_prop_ids.end();
      }

      /// @brief Equality comparison operator
      auto operator==(const PropertyFilter& other) const -> bool
      {
         return m_filter_name == other.m_filter_name and 
                m_prop_ids    == other.m_prop_ids    and 
                m_compare_pred(m_compare_val, other.m_compare_val) == other.m_compare_pred(m_compare_val, other.m_compare_val);
      }

      PropertyFilter() = default;
      ~PropertyFilter() noexcept = default;
      PropertyFilter(const PropertyFilter&) = default;
      PropertyFilter(PropertyFilter&&) = default;
      PropertyFilter& operator=(const PropertyFilter&) = default;
      PropertyFilter& operator=(PropertyFilter&&) = default;

   private:
      std::string m_filter_name{};
      MatchProps  m_prop_ids{};
      PropertyVal m_compare_val{};
      ComparePred m_compare_pred{};
   };

} // namespace ctb::detail