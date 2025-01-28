
#include "grids/GridTableMgr.h"
#include "grids/GridTableWineList.h"

#include "ctb/functors.h"
#include "ctb/data/table_data.h"
#include "ctb/data/WineListEntry.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb
{
   using namespace magic_enum;
   using namespace ctb::data;

   GridTableMgr::GridTablePtr GridTableMgr::getGridTable(GridTableId tbl)
   {
      if (m_grid_tables.contains(tbl))
         return m_grid_tables[tbl];

      Overloaded TableFactory{
         [&](enum_constant<GridTableId::WineList>) 
            { 
               auto table_data = loadTableData<WineListData>(m_data_folder, TableId::List);
               if (!table_data)
               {
                  throw table_data.error();
               }
               auto grid = std::make_shared<GridTableWineList>(std::move(table_data.value())); 
               return grid;
            }
      };
      auto grid_table = enum_switch(TableFactory, tbl);
      m_grid_tables[tbl] = grid_table;
      return grid_table;
   }

}
