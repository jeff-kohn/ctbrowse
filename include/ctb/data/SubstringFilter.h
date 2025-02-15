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
#include <format>
#include <string_view>
#include <vector>


namespace ctb::data
{

   /// @brief implements a substring-matching filter for a table entry/record
   ///
   template <TableEntry T>
   struct SubStringFilter
   {
   public:
      using Prop = T::Prop;

      /// @brief the substring to search for.
      ///
      std::string search_value{};

      /// @brief the properties to search for.
      ///
      std::vector<Prop> search_props{};


      /// @brief  function operator used to perform the substring search. 
      ///
      /// this function will check each specified property to see if it contains
      /// the search substring, returning true if a match was round. case-neutral
      /// search is used
      /// 
      bool operator()(const T& rec) const
      {
         for (auto prop : search_props)
         {
            auto val_result = rec.getProperty(prop);
            if (val_result.has_value())
            {
               return val_result->asString().contains(search_value);
            }              
         }
         return false;
      }

   };


} // namespace ctb::data