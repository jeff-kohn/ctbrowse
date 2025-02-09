/*******************************************************************
 * @file GridTableFilterMgr.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright (c) 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "interfaces/GridTableFilter.h"

#include <map>
#include <optional>
#include <string>


namespace ctb::app
{

   class GridTableFilterMgr
   {
   public:

      auto getFilters() const
      {
      }

      auto getFilterValues(int prop_index) const
      {

      }

   private:
      // map filters to the property index they work on.
      using PropFilterMap = std::map<int, GridTableFilter>;
      PropFilterMap m_filters{};
   };

} // namespace ctb::app