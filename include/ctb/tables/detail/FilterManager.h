/*******************************************************************
* @file FilterManager.h
*
* @brief Defines the FilterManager template class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/PropertyFilter.h"
#include <map>

namespace ctb::detail
{
   /// @brief class to manage a set of property filters applied to a dataset
   /// 
   /// Filters are uniquely identified by their key. There are constrained overloads that allow
   /// using a string_view as lookup for string key when appropriate.
   ///  
   /// For a dataset record to be a match, it must match each filter contained in the collection.
   template<typename FilterT, typename KeyT, EnumType PropT, PropertyMapType PropMapT>
   class FilterManager  
   {
   public:
      using Key              = KeyT;
      using Prop             = PropT;
      using PropertyMap      = PropMapT;
      using PropertyVal      = PropertyMap::mapped_type;
      using Filter           = FilterT;
      using MaybeFilter      = std::optional<Filter>;
      using FilterMap        = std::map<Key, Filter, std::less<>>;

      /// @brief Adds a filter to the collection if it does not already exist.
      /// @return true if the filter was successfully added; false if a filter with the same name already exists.
      auto addFilter(Key key, Filter filter) -> bool
      {
         return m_filters.try_emplace(std::move(key), std::move(filter)).second;
      }

      /// @brief Adds a filter to the collection if it does not already exist, with heterogeneous lookup
      /// @return true if the filter was successfully added; false if a filter with the same name already exists.
      template<StringOrStringViewType KeyValT> requires StringOrStringViewType<Key>
      auto addFilter(KeyValT&& key, Filter filter) -> bool
      {
         return m_filters.try_emplace(std::forward<KeyValT>(key), std::move(filter)).second;
      }

      /// @brief Remove the specified filter from the list
      /// @return true if removed, false if not found.
      auto removeFilter(const Key& key) -> bool
      {
         return m_filters.erase(key) > 0;
      }

      /// @brief Remove the specified filter from the list, with heterogeneous lookup
      /// @return true if removed, false if not found.
      template<StringOrStringViewType KeyValT> requires StringOrStringViewType<Key>
      auto removeFilter(KeyValT&& key) -> bool
      {
         return m_filters.erase(std::forward<KeyValT>(key)) > 0;
      }

      /// @brief Remove all filters from this object
      /// @return true if at least one filter was removed, false if there were no filters
      auto clear() -> bool
      {
         bool removed = !m_filters.empty();
         m_filters.clear();
         return removed;
      }

      /// @brief Checks if a filter with the specified name exists.
      /// @return true if a filter with the matching key exists, false otherwise
      auto hasFilter(const Key& key) const -> bool
      {
         return m_filters.contains(key);
      }

      /// @brief Checks if a filter with the specified name exists, with heterogeneous lookup
      /// @return true if a filter with the matching key exists, false otherwise
      template<StringOrStringViewType KeyValT> requires StringOrStringViewType<Key>
      auto hasFilter(KeyValT&& key) const -> bool
      {
         return m_filters.contains(std::forward<KeyValT>(key));
      }

      /// @brief Retrieves a filter for the specified key, if it exists
      /// @return the requested filter if found, or an empty std::nullopt if not.
      auto getFilter(const Key& key) const -> MaybeFilter
      {
         auto   it  = m_filters.find(key);
         return it == m_filters.end() ? MaybeFilter{} : MaybeFilter{ it->second };
      }

      /// @brief Retrieves a filter for the specified key, if it exists, with heterogeneous lookup
      /// @return the requested filter if found, or an empty std::nullopt if not.
      template<StringOrStringViewType KeyValT> requires StringOrStringViewType<Key>
      auto getFilter(KeyValT&& key) const -> MaybeFilter
      {
         auto   it  = m_filters.find(std::forward<KeyValT>(key));
         return it == m_filters.end() ? MaybeFilter{} : MaybeFilter{ it->second };
      }

      /// @brief returns the number of active property filters we have.
      auto activeFilters() const -> int
      {
         return static_cast<int>(m_filters.size());
      }

      /// @brief operator[] for getting a filter if it exists or adding a new default initialized one otherwise
      /// @return a reference to the requested (or new) Filter.
      template <typename Self>
      auto&& operator[](this Self&& self, const Key& key) 
      {
         // can't use operator[] or .at() with heterogenous lookup. This approach saves a string copy when value already exists
         auto it = std::forward<Self>(self).m_filters.find(key);
         if (it == std::forward<Self>(self).m_filters.end())
         {
            auto result = std::forward<Self>(self).m_filters.try_emplace(key, Filter{});
            assert(result.second);
            return result.first->second;
         }
         return it->second;
      }
      
      /// @brief returns true if the record is a match 
      auto operator()(const PropertyMap& rec) const -> bool
      {
         // note we're looking for a miss, not a match, because we can return 
         // false on first miss but have to match all filters before we can return true
         for (auto& filter : vws::values(m_filters))
         {
            if (false == filter(rec))
               return false;
         }
         return true;
      }

      FilterManager() noexcept = default;
      ~FilterManager() noexcept = default;
      FilterManager(const FilterManager&) = default;
      FilterManager(FilterManager&&) = default;
      FilterManager& operator=(const FilterManager&) = default;
      FilterManager& operator=(FilterManager&&) = default;

   private:
      FilterMap m_filters;
   };


} // namespace ctb::detail