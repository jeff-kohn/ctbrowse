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
#include "ctb/tables/TastingNotesTraits.h"
#include "ctb/tables/WineListTraits.h"

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_switch.hpp>


namespace ctb
{
   using namespace magic_enum;


   namespace
   {
      template<typename TableT>
      auto getOrThrow(const fs::path& folder, TableId tbl_id) -> DatasetPtr
      {
         auto result = loadTableData<TableT>(folder, tbl_id);
         if (!result)
				throw result.error();

			return CtDataset<TableT>::create(std::move(result.value()));
      }
   }

   auto CtDatasetLoader::getDataset(TableId tbl) -> DatasetPtr
   {
      Overloaded TableFactory{
         [this](enum_constant<TableId::List> tbl_id)  -> DatasetPtr
            { 
					return getOrThrow<WineListTable>(m_data_folder, tbl_id);
            },

         [this](enum_constant<TableId::Pending> tbl_id) -> DatasetPtr
            { 
               return getOrThrow<PendingWineTable>(m_data_folder, tbl_id);
            }, 

         [this](enum_constant<TableId::Consumed> tbl_id) -> DatasetPtr
            { 
               return getOrThrow<ConsumedWineTable>(m_data_folder, tbl_id);
            }, 

         [this](enum_constant<TableId::Availability> tbl_id) -> DatasetPtr
            { 
               return getOrThrow<ReadyToDrinkTable>(m_data_folder, tbl_id);
            },

         [this](enum_constant<TableId::Purchase> tbl_id) -> DatasetPtr
            {
               return getOrThrow<PurchasedWineTable>(m_data_folder, tbl_id);
            },

         [this](enum_constant<TableId::Notes> tbl_id) -> DatasetPtr
            {
               return getOrThrow<TastingNotesTable>(m_data_folder, tbl_id);
            }
      };
      return enum_switch(TableFactory, tbl);
   }

}  // ctb