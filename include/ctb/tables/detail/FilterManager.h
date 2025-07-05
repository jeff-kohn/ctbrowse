/*******************************************************************
* @file FilterManager.h
*
* @brief Defines the FilterManager template class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
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
      using ChangeCallback   = std::function<void(void)>;
      

      /// @brief Adds a filter to the collection if it does not already exist.
      /// @return true if the filter was successfully added; false if a filter with the same filter_name already exists.
      auto addFilter(Key key, Filter filter) -> bool
      {
         if (m_filters.try_emplace(std::move(key), std::move(filter)).second)
         {
            notifyChange();
            return true;
         }  
         return false;
      }

      /// @brief Replace an existing filter, or add it if it does not already exist.
      void replaceFilter(Key key, Filter filter)
      {
         m_filters[key] = std::move(filter);
         notifyChange();
      }

      template<rng::input_range Rng> //requires std::same_as<rng::range_value_t<Rng>, FilterMap::value_type>
      void assignFilters(Rng&& rng)
      {
         m_filters.clear();
         m_filters.insert_range(std::forward<Rng>(rng));
         notifyChange();
      }


      /// @brief Remove the specified filter from the list
      /// @return true if removed, false if not found.
      auto removeFilter(const Key& key) -> bool
      {
         if ( m_filters.erase(key))
         {
            notifyChange();
            return true;
         }
         return false;
      }

      /// @brief Remove the specified filter from the list, with heterogeneous lookup
      /// @return true if removed, false if not found.
      template<StringOrStringViewType KeyValT> requires StringOrStringViewType<Key>
      auto removeFilter(KeyValT&& key) -> bool
      {
         if (m_filters.erase(std::forward<KeyValT>(key)))
         {
            notifyChange();
            return true;
         }
         return false;      
      }

      /// @brief Remove all filters from this object
      /// @return true if at least one filter was removed, false if there were no filters
      auto clear() -> bool
      {
        if (empty())
           return false;

        m_filters.clear();
        notifyChange();
        return true;
      }

      /// @brief Checks if a filter with the specified filter_name exists.
      /// @return true if a filter with the matching key exists, false otherwise
      auto hasFilter(const Key& key) const -> bool
      {
         return m_filters.contains(key);
      }

      /// @brief Checks if a filter with the specified filter_name exists, with heterogeneous lookup
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

      /// @return - true if there is at least one active filter, false otherwise.
      auto empty() const -> bool
      {
         return m_filters.empty();
      }

      /// @return The number of filters in this manager
      auto size() const -> size_t
      {
         return m_filters.size();
      }

      /// @brief Retrieve a view on all active filters.
      /// @return A view representing all active filters (could be empty)
      auto activeFilters() const
      {
         return vws::all(m_filters);
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

      /// @brief Subscribe to callback to be notified when a filter is added/changed/removed/etc
      template <typename Callable> requires std::invocable<Callable> and std::same_as<void, std::invoke_result_t<Callable>> 
      void subscribeChanges(Callable&& callable)
      {
         m_callback = std::forward<Callable>(callable);
      }

      /// @brief Unsubscribe from change notification
      void unsubscribeChanges()
      {
         m_callback = {};
      }

      /// @brief Construct a FilterManager and use the provided callback for change notifications
      /// @throw ctb::Error if dataset == nullptr
      FilterManager(ChangeCallback callback) noexcept(false) : m_callback(callback)
      {}

      FilterManager() noexcept                  = default;
      FilterManager(FilterManager&&)            = default;
      FilterManager& operator=(FilterManager&&) = default;
      ~FilterManager() noexcept                 = default;

      FilterManager(const FilterManager&)            = delete;
      FilterManager& operator=(const FilterManager&) = delete;

   private:
      FilterMap      m_filters{};
      ChangeCallback m_callback{};

      void notifyChange()
      {
         if (m_callback)
            m_callback();
      }
   };


} // namespace ctb::detail