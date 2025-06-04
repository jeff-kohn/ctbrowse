#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/PropertyFilter.h"

#include <map>

namespace ctb::detail
{
   /// @brief class to manage a set of property filters applied to a dataset
   ///  
   /// For a dataset record to be a match, it must match each filters contained in the collection.
   /// Filters are uniquely identified by their name (case-sensitive).
   template<EnumType PropT, PropertyMapType PropMapT>
   class PropertyFilterMgr
   {
   public:
      using Prop             = PropT;
      using PropertyMap      = PropMapT;
      using Property         = PropertyMap::mapped_type;
      using Filter           = PropertyFilter<Prop, PropertyMap>;
      using MaybePropFilter  = std::optional<Filter>;

      // use a string identifier, we can't use PropT as key because some filters apply to more than
      // one property. The key will be the filter_name property from the filter, and callers can also
      // use it for a user-facing name/label for filter.
      using FilterMap = std::map<std::string, Filter, std::less<>>;

      /// @brief Adds a filter to the collection if it does not already exist.
      /// @return true if the filter was successfully added; false if a filter with the same name already exists.
      auto addFilter(Filter filter) -> bool
      {
         return m_filters.try_emplace(std::string{ filter.filter_name }, std::move(filter)).second;
      }

      /// @brief Remove filter matching the specified name
      /// @return true if removed, false if not found.
      auto removeFilter(std::string_view filter_name) -> bool
      {
         return m_filters.erase(filter_name) > 0;
      }

      /// @brief Remove all filters from this object
      /// @return true if at least one filter was removed, false if there were no filters
      auto removeAllFilters() -> bool
      {
         bool removed = !m_filters.empty();
         m_filters.clear();
         return removed;
      }

      /// @brief Checks if a filter with the specified name exists.
      auto hasFilter(std::string_view filter_name) const -> bool
      {
         return m_filters.contains(filter_name);
      }

      /// @brief Retrieves a property filter by its name, if it exists.
      /// @return the requested filter if found, or an empty std::nullopt if not.
      auto getFilter(std::string_view filter_name) const -> MaybePropFilter
      {
         auto   it  = m_filters.find(filter_name);
         return it == m_filters.end() ? MaybePropFilter{} : MaybePropFilter{ it->second };
      }

      /// @brief returns the number of active property filters we have.
      auto activeFilters() const -> int
      {
         return static_cast<int>(m_filters.size());
      }

      /// @brief [] operator for add-or-insert
      /// @return reference to the requested object
      template <typename Self>
      auto&& operator[](this Self&& self, std::string_view filter_name)
      {
         // can't use operator[] or .at() with heterogenous lookup. This approach saves a string copy when value already exists
         auto it = std::forward<Self>(self).m_filters.find(filter_name);
         if (it == std::forward<Self>(self).m_filters.end())
         {
            return std::forward<Self>(self).m_filters[std::string{filter_name}];
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

      PropertyFilterMgr() noexcept = default;
      ~PropertyFilterMgr() noexcept = default;
      PropertyFilterMgr(const PropertyFilterMgr&) = default;
      PropertyFilterMgr(PropertyFilterMgr&&) = default;
      PropertyFilterMgr& operator=(const PropertyFilterMgr&) = default;
      PropertyFilterMgr& operator=(PropertyFilterMgr&&) = default;

   private:
      FilterMap m_filters;
   };


} // namespace ctb::detail