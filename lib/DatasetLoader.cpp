/*******************************************************************
 * @file DatasetLoader.cpp
 *
 * @brief Header file for the DatasetLoader class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "ctb/tables/PendingWineTable.h"
#include "ctb/tables/WineListTable.h"

#include "ctb/model/DatasetLoader.h"
#include "ctb/model/CtDataModel.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb
{
   using namespace magic_enum;


   auto DatasetLoader::getDataset(TableId tbl) -> DatasetPtr
   {
      Overloaded TableFactory{
         [this](enum_constant<TableId::List>)  -> DatasetPtr
            { 
               auto table_data = loadTableData<WineListTable>(m_data_folder, TableId::List);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataModel<WineListTable>::create(std::move(table_data.value()));
            },

         [this](enum_constant<TableId::Pending>) -> DatasetPtr
            { 
               auto table_data = loadTableData<PendingWineTable>(m_data_folder, TableId::Pending);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataModel<PendingWineTable>::create(std::move(table_data.value()));
            } 
      };
      return enum_switch(TableFactory, tbl);
   }

}  // ctb