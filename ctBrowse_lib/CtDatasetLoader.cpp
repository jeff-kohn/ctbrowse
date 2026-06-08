/*******************************************************************
 * @file CtDatasetLoader.cpp
 *
 * @brief Header file for the CtDatasetLoader class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "ctb/model/CtDatasetLoader.h"
#include "ctb/model/CtDataset.h"

#include "ctb/tables/BottleInventoryTraits.h"
#include "ctb/tables/ConsumedWineTraits.h"
#include "ctb/tables/PendingWineTraits.h"
#include "ctb/tables/PrivateNotesTraits.h"
#include "ctb/tables/ProReviewsCacheTraits.h"
#include "ctb/tables/PurchasedWineTraits.h"
#include "ctb/tables/ReadyToDrinkTraits.h"
#include "ctb/tables/TaggedWinesTraits.h"
#include "ctb/tables/TastingNotesTraits.h"
#include "ctb/tables/WineListTraits.h"

#include <optional>

namespace ctb
{

   namespace
   {
      template<typename TableT>
      auto getOrThrow(const fs::path& folder, TableId tbl_id) -> DatasetPtr
      {
         auto result = loadTableData<TableT>(folder, tbl_id);
         if (!result)
            throw Error{ result.error() };

			return CtDataset<TableT>::create(std::move(result.value()));
      }
   }

   auto CtDatasetLoader::getDataset(TableId tbl) -> DatasetPtr
   {
      switch (tbl)
      {
         case TableId::List:          return getOrThrow<WineListTable>(m_data_folder, tbl);
         case TableId::Pending:       return getOrThrow<PendingWineTable>(m_data_folder, tbl);
         case TableId::Consumed:      return getOrThrow<ConsumedWineTable>(m_data_folder, tbl);
         case TableId::Availability:  return getOrThrow<ReadyToDrinkTable>(m_data_folder, tbl);
         case TableId::Purchase:      return getOrThrow<PurchasedWineTable>(m_data_folder, tbl);
         case TableId::Tag:           return getOrThrow<TaggedWinesTable>(m_data_folder, tbl);
         case TableId::Inventory:     return getOrThrow<BottleInventoryTable>(m_data_folder, tbl);
         case TableId::PrivateNotes:  return getOrThrow<PrivateNotesTable>(m_data_folder, tbl);
		   case TableId::Notes:         return getOrThrow<TastingNotesTable>(m_data_folder, tbl);
		   default:
	         throw Error{"Table not found."};
      };
   }


   auto CtDatasetLoader::getProReviewsCache() -> std::optional<ProReviewsCache>
   {
      auto result = loadTableData<ProReviewsCacheTable>(m_data_folder, TableId::Availability);
      if (!result)
         return {};

      return ProReviewsCache{ result.value() };
   }

}  // namespace ctb
