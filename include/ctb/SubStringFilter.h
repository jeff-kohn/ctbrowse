/*******************************************************************
 * @file SubStringFilter.h
 *
 * @brief defines the template class SubStringFilter
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <boost/algorithm/string.hpp>
#include <expected>
#include <string_view>
#include <vector>


namespace ctb
{

   /// @brief implements a substring-matching filter for a table entry/record
   ///
   template <CtRecord RecordTypeT>
   struct SubStringFilter
   {
   public:
      using Record = RecordTypeT;
      using PropId = Record::PropId;

      /// @brief the substring to search for.
      ///
      std::string search_value{};

      /// @brief the properties to search in.
      ///
      std::vector<PropId> search_props{};

      /// @brief  function operator used to perform the substring search. 
      ///
      /// this function will check each specified property to see if it contains
      /// the search substring, returning true if a match was round. case-neutral
      /// search is used
      /// 
      bool operator()(const Record& rec) const
      {
         for (auto prop : search_props)
         {
            auto val = rec.getProperty(prop);

            if ( val.hasString() and boost::icontains(val.asStringView(), search_value) )
               return true;
            
            if (boost::icontains(val.asString(), search_value))
               return true;
         }
         return false;
      }

   };


} // namespace ctb