/*******************************************************************7
* @file PropFilterMgrString.h
*
* @brief defines the template class PropFilterMgrString
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/PropertyFilterString.h"

#include <map>
#include <set>
#include <string>
#include <string_view>


namespace ctb
{
   


   /// @brief class to manage property filters for a data table.
   /// 
   /// this class currently only works with strings. Numeric table properties
   /// will be converted to string for filtering purposes. Not ideal for 
   /// performance, but most of our filters are text base. Will revisit this later.
   /// 
   template <CtRecord RecordTypeT>
   class PropFilterMgrString
   {
   public:
      using RecordType   = RecordTypeT;
      using StringFilter = PropertyFilterString<RecordType>;
      using PropId       = RecordType::PropId;


      /// @brief add a match value for the specified column filter.
      /// @return true if successful, false if filter value already existed or could not be added.
      /// 
      bool addFilter(PropId prop_id, std::string_view match_value)
      {
         auto& filter = m_filters[prop_id];

         // match values contains ValueWrapper, which will gladly accept string_view and cause issues
         // so we need to explicitly insert a std::string
         std::string match_str{ match_value };
         filter.match_values.insert(match_str);

         // note we always assign the filter object's prop-id because if the filter
         // is being default-constructed on-demand it won't have the correct prop_id.
         filter.prop_id = prop_id;

         return true;
      }


      /// @brief remove a match value for the specified column filter
      /// @return true if removed, false if not found.1
      /// 
      bool removeFilter(PropId prop_id, std::string_view match_value)
      {
         bool ret_val{ false };

         auto filt_it = m_filters.find(prop_id);
         if (filt_it != m_filters.end())
         {
            auto& filter = filt_it->second;

            ret_val = filter.match_values.erase(match_value);
            if (filter.match_values.empty())
            {
               m_filters.erase(filt_it);
            }
         }

         return ret_val;
      }


      /// @brief check if a record matches all of our filters
      /// @return true if each PropertyFilterString matched the record, false  
      ///         if the record failed to match one or more filters.
      /// 
      bool isMatch(const RecordType& rec) const
      {
         for (auto&& filter : vws::values(m_filters))
         {
            if ( !filter(rec) )
               return false;
         }
         return true;
      }


      /// @brief returns the number of active property filters we have.
      /// 
      int activeFilters() const
      {
         return static_cast<int>(rng::count_if( m_filters | vws::values, [](auto&& filter) { return filter.match_values.size(); } ));
      }

      
      /// @brief retrieve a list of possible filter values for the given property in the table
      ///
      template<rng::input_range RowsT, typename PropEnumT> 
      static StringSet getFilterMatchValues(const RowsT& rows, PropEnumT prop_id) 
      {
         StringSet result{};
         for (auto& row : rows)
         {
            auto val = row[prop_id];
            if (val)
            {
               result.emplace(val.asString());
            }
         }
         return result;
      }


   private:
      std::map<PropId, StringFilter> m_filters{};
   };

} // namespace ctb