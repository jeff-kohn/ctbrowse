/*******************************************************************7
* @file MultiMatchPropertyFilterMgr.h
*
* @brief defines the template class MultiMatchPropertyFilterMgr
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/MultiMatchPropertyFilter.h"

#include <boost/unordered/unordered_flat_map.hpp>
#include <string>
#include <string_view>


namespace ctb::detail
{
   
   /// @brief Class to manage multi-match property filters for a dataset.
   /// 
   /// 
   template<EnumType PropT, PropertyMapType PropMapT>
   class MultiMatchPropertyFilterMgr
   {
   public:
      using Prop          = PropT;
      using PropertyMap   = PropMapT;
      using Property = PropertyMap::mapped_type;
      using Filter        = MultiMatchPropertyFilter<Prop, PropertyMap>;
      using MatchValues   = Filter::MatchValues;

      /// @brief add a match value for the specified column filter.
      /// @return true if successful, false if filter value already existed or could not be added.
      /// 
      bool addFilter(Prop prop_id, const Property& match_value)
      {
         auto& filter = m_filters[prop_id];

         // note we always assign the filter object's prop-id because if the filter
         // is being default-constructed on-demand it won't have the correct prop_id.
         filter.prop_id = prop_id;
         return filter.match_values.insert(match_value).second;
      }

      /// @brief remove a match value for the specified column filter
      /// @return true if removed, false if not found.
      /// 
      bool removeFilter(Prop prop_id, const Property& match_value)
      {
         bool ret_val{ false };

         auto filt_it = m_filters.find(prop_id);
         if (filt_it != m_filters.end())
         {
            auto& filter = filt_it->second;
            ret_val = filter.match_values.erase(match_value);

            // if we're removing the last match value, remove the filter altogether so we don't keep checking it
            if (filter.match_values.empty())
            {
               m_filters.erase(filt_it);
            }
         }
         return ret_val;
      }

      /// @brief check if a record matches all of our filters
      /// @return true if each PropFilterString matched the record, false  
      ///         if the record failed to match one or more filters. Will
      ///         also return true if there are no active filters.
      /// 
      bool operator()(const PropertyMap& rec) const
      {
         if (activeFilters())
         {
            for (auto&& filter : vws::values(m_filters))
            {
               if ( !filter(rec) )
                  return false;
            }
         }
         return true;
      }

      /// @brief returns the number of active property filters we have.
      /// 
      int activeFilters() const
      {
         return static_cast<int>(m_filters.size());
      }
            
   private:
      using FilterMap = boost::unordered_flat_map<Prop, Filter>;
      FilterMap m_filters{};
   };

} // namespace ctb::detail
