/*******************************************************************
 * @file DatasetLoader.cpp
 *
 * @brief Header file for the DatasetLoader class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "model/DatasetLoader.h"
#include "model/CtDataModel.h"

#include <ctb/utility.h>
#include <ctb/table/table_data.h>
#include <ctb/WineListTraits.h>
#include <ctb/PendingWineTraits.h>

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb::app
{
   using namespace magic_enum;

   auto DatasetLoader::getDataset(TableId tbl) -> DatasetPtr
   {
      Overloaded TableFactory{
         [this](enum_constant<TableId::List>)  -> DatasetPtr
            { 
               auto table_data = loadTableData<WineListDataset>(m_data_folder, TableId::List);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataModel<WineListDataset>::create(std::move(table_data.value()));
            },

/*         [this](enum_constant<TableId::List>)  -> DatasetPtr
         { 
            auto table_data = loadTableData<PendingWineDataset>(m_data_folder, TableId::Pending);
            if (!table_data)
            {
               throw table_data.error();
            }
            return CtDataModel<PendingWineDataset>::create(std::move(table_data.value()));
         } */     
      };
      return enum_switch(TableFactory, tbl);
   }

}  // ctb::app
