/*******************************************************************
 * @file PropStringFilter.h
 *
 * @brief defines the template class PropStringFilter
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
//#include "ctb/table/table_data.h"
//#include "ctb/table/TableProperty.h"
//#include "ctb/utility.h"
//
//#include <string>
//#include <set>
//#include <variant>

namespace ctb
{

   /// @brief class that can be used to filter table entry records based
   ///        on one or more match values for a given property.
   /// 
   template <CtRecord RecordTypeT>
   struct PropStringFilter
   {
      // some types we borrow from our template parameters
      using Record = RecordTypeT;
      using PropId  = Record::PropId;

      /// @brief the property that we're filtering against
      ///
      PropId prop_id{};

      /// @brief the possible values to match against
      ///
      StringSet match_values{};

      /// @brief returns true if the specified table entry is a match
      ///
      bool operator()(const Record& rec) const
      {
         if (match_values.empty())
            return true;

         auto prop_val = rec[prop_id];
         if (prop_val.isNull())
            return false;

        // asStringView() would be faster but wouldn't allow searching the non-text properties
        return match_values.find(prop_val.asString()) != match_values.end();
      }
   };


} // namespace ctb
