#pragma once

#include "App.h"
#include "interfaces/GridTable.h"

#include <ctb/nullable_types.h>
#include <vector>
#include <set>

namespace ctb::app
{
   class GridTable;

   struct GridTableFilterInfo
   {
      /// @brief this is the display name for the filter type
      ///
      std::string filter_type{};

      /// @brief this is the property index the filter will be applied to in the dataset
      ///
      int prop_index{};
   };


   struct GridTableFilter
   {
      /// @brief type alias for a unique list of string values to filter on.
      using FilterValues = std::set<std::string>;
      using NullableFilterValues = std::optional<FilterValues>;

      /// @brief contains a description of the filter
      ///
      GridTableFilterInfo filter_info{};

      /// @brief this is the list of values that column values will be matched against.
      ///
      NullableFilterValues filter_values{};

      void getFilterValues(GridTable* grid_table){}
   };






} // namespace ctb::app