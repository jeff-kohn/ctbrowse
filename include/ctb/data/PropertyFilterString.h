/*******************************************************************
 * @file PropertyFilterString.h
 *
 * @brief defines the template class PropertyFilterString
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/data/table_data.h"
#include "ctb/data/TableProperty.h"
#include "ctb/functors.h"

#include <string>
#include <set>
#include <variant>

namespace ctb::data
{

   /// @brief class that can be used to filter table entry records based
   ///        on one or more values for a given property.
   /// 
   template <TableEntry RecordType>
   struct PropertyFilterString
   {
      // some types we borrow from our template parameter
      using Prop  = RecordType::Prop;


      /// @brief the property that we're filtering against
      ///
      Prop prop_id{};


      /// @brief the possible values to match against
      ///
      StringSet match_values{};


      /// @brief returns true if the specified table entry is a match
      ///
      bool operator()(const RecordType& rec) const
      {
         auto prop_result = rec[prop_id];
         if (prop_result)
         {
            return match_values.find(prop_result->asString() ) != match_values.end();
         }
         return false;
      }
   };



} // namespace ctb::data
