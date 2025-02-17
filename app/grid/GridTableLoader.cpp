/*******************************************************************
 * @file GridTableLoader.cpp
 *
 * @brief Header file for the GridTableLoader class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "grid/GridTableLoader.h"
#include "grid/GridTableWineList.h"

#include <ctb/functors.h>
#include <ctb/table_data.h>
#include <ctb/WineListTraits.h>

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb::app
{
   using namespace magic_enum;

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
