#pragma once

#include "App.h"
#include "interfaces/GridTable.h"

#include <ctb/nullable_types.h>
#include <string>
#include <set>


namespace ctb::app
{
   class GridTable;

   struct GridTableFilter
   {
      /// @brief this is the display name for the filter type
      ///
      std::string filter_type{};

      /// @brief this is the property index the filter will be applied to in the dataset
      ///
      int prop_index{};

      /// @brief retrieve a list of available values in the table for this filter
      ///
      std::set<std::string> getMatchValues(GridTable* grid_table);
   };

} // namespace ctb::app