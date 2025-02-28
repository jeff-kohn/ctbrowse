/*******************************************************************
 * @file PropertyFilterString.h
 *
 * @brief defines the template class PropertyFilterString
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/table_data.h"
#include "ctb/TableProperty.h"
#include "ctb/utility.h"

#include <string>
#include <set>
#include <variant>

namespace ctb
{

   /// @brief class that can be used to filter table entry records based
   ///        on one or more values for a given property.
   /// 
   template <CtRecord RecordTypeT>
   struct PropertyFilterString
   {
      // some types we borrow from our template parameters
      using RecordType = RecordTypeT;
      using PropId  = RecordType::PropId;


      /// @brief the property that we're filtering against
      ///
      PropId prop_id{};


      /// @brief the possible values to match against
      ///
      StringSet match_values{};


      /// @brief returns true if the specified table entry is a match
      ///
      bool operator()(const RecordType& rec) const
      {
         if (match_values.empty())
            return true;

         auto prop_val = rec[prop_id];
         if (prop_val.isNull())
            return false;
         
        return match_values.find(prop_val.asStringView()) != match_values.end();
      }
   };



} // namespace ctb
