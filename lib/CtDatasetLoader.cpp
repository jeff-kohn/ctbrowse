/*******************************************************************
 * @file CtDatasetLoader.cpp
 *
 * @brief Header file for the CtDatasetLoader class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "ctb/tables/PendingWineTraits.h"
#include "ctb/tables/WineListTraits.h"

#include "ctb/model/CtDatasetLoader.h"
#include "ctb/model/CtDataset.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb
{
   using namespace magic_enum;


   auto CtDatasetLoader::getDataset(TableId tbl) -> DatasetPtr
   {
      Overloaded TableFactory{
         [this](enum_constant<TableId::List>)  -> DatasetPtr
            { 
               auto table_data = loadTableData<WineListTable>(m_data_folder, TableId::List);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataset<WineListTable>::create(std::move(table_data.value()));
            },

         [this](enum_constant<TableId::Pending>) -> DatasetPtr
            { 
               auto table_data = loadTableData<PendingWineTable>(m_data_folder, TableId::Pending);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataset<PendingWineTable>::create(std::move(table_data.value()));
            } 
      };
      return enum_switch(TableFactory, tbl);
   }

}  // ctb