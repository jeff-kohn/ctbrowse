/*******************************************************************
 * @file CtDatasetLoader.cpp
 *
 * @brief Header file for the CtDatasetLoader class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "ctb/model/CtDatasetLoader.h"
#include "ctb/model/CtDataset.h"

#include "ctb/tables/ConsumedWineTraits.h"
#include "ctb/tables/PendingWineTraits.h"
#include "ctb/tables/PurchasedWineTraits.h"
#include "ctb/tables/ReadyToDrinkTraits.h"
#include "ctb/tables/WineListTraits.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb
{
   using namespace magic_enum;


   auto CtDatasetLoader::getDataset(TableId tbl) -> DatasetPtr
   {
      Overloaded TableFactory{
         [this](enum_constant<TableId::List> tbl_id)  -> DatasetPtr
            { 
               auto table_data = loadTableData<WineListTable>(m_data_folder, tbl_id);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataset<WineListTable>::create(std::move(table_data.value()));
            },

         [this](enum_constant<TableId::Pending> tbl_id) -> DatasetPtr
            { 
               auto table_data = loadTableData<PendingWineTable>(m_data_folder, tbl_id);
               if (!table_data)
               {
                  throw table_data.error();
               }
               return CtDataset<PendingWineTable>::create(std::move(table_data.value()));
            }, 
         [this](enum_constant<TableId::Consumed> tbl_id) -> DatasetPtr
         { 
            auto table_data = loadTableData<ConsumedWineTable>(m_data_folder, tbl_id);
            if (!table_data)
            {
               throw table_data.error();
            }
            return CtDataset<ConsumedWineTable>::create(std::move(table_data.value()));
         }, 
         [this](enum_constant<TableId::Availability> tbl_id) -> DatasetPtr
         { 
            auto table_data = loadTableData<ReadyToDrinkTable>(m_data_folder, tbl_id);
            if (!table_data)
            {
               throw table_data.error();
            }
            return CtDataset<ReadyToDrinkTable>::create(std::move(table_data.value()));
         },
         [this](enum_constant<TableId::Purchase> tbl_id) -> DatasetPtr
         {
            auto table_data = loadTableData<PurchasedWineTable>(m_data_folder, tbl_id);
            if (!table_data)
            {
               throw table_data.error();
            }
            return CtDataset<PurchasedWineTable>::create(std::move(table_data.value()));
         }
      };
      return enum_switch(TableFactory, tbl);
   }

}  // ctb