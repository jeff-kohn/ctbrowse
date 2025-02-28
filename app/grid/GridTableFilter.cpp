#include "App.h"
#include "grid/GridTableFilter.h"
#include "interfaces/GridTable.h"

namespace ctb::app
{
   // This needs to go here because the header can't have a dependency on GridTable interface 
   StringSet GridTableFilter::getMatchValues(GridTable* grid_table) const
   {
      return grid_table->getFilterMatchValues(m_prop_index);
   }

} // ctb::app