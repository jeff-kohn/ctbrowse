#include "App.h"
#include "model/CtStringFilter.h"
#include "interfaces/IDataset.h"

namespace ctb::app
{
   // This needs to go here because the header can't have a dependency on GridTable interface 
   StringSet CtStringFilter::getMatchValues(GridTable* grid_table) const
   {
      return grid_table->getFilterMatchValues(m_prop_index);
   }

} // ctb::app