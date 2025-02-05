
#include "grid/GridTableLoader.h"
#include "grid/GridTableWineList.h"

#include "ctb/functors.h"
#include "ctb/data/table_data.h"
#include "ctb/data/WineListEntry.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb::app
{
   using namespace magic_enum;
   using namespace ctb::data;

   GridTablePtr GridTableLoader::getGridTable(GridTableId tbl)
   {
      Overloaded TableFactory{
         [this](enum_constant<GridTableId::WineList>) 
            { 
               auto table_data = loadTableData<WineListData>(m_data_folder, TableId::List);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return GridTableWineList::create(std::move(table_data.value())); 
            }
      };
      return enum_switch(TableFactory, tbl);
   }

}  // ctb::app
