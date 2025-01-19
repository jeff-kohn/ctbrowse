#include "App.h"
#include "CtGridTable.h"
#include "cts/data/DataManager.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>
#include <cassert>

namespace cts
{
   using namespace magic_enum;
   using namespace cts::data;

   CtGridTableMgr::GridTablePtr CtGridTableMgr::getGridTable(GridTable tbl)
   {
      if (m_tables.contains(tbl))
         return m_tables[tbl];

      DataManager mgr{ wxGetApp().userDataFolder() };

      Overloaded TableFactory{
         [&mgr](enum_constant<GridTable::WineList>) 
            { 
               auto table_data = mgr.getWineList();
               auto grid_ptr = std::make_shared<CtGridTable<WineListData> >(); 
               grid_ptr->loadTable(std::move(table_data));
               return grid_ptr;
            }
      };
      auto table_data = enum_switch(TableFactory, tbl);
      m_tables[tbl] = table_data;
      return table_data;
   }

}
