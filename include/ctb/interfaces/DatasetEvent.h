/*******************************************************************
 * @file DatasetEvent.h
 *
 * @brief Header file defining DatasetBase event class and related 
          source/sink interfaces.
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/interfaces/IDataset.h"

namespace ctb
{
   /// @brief categorizes the different notification events supported by this interface
   ///
   struct DatasetEvent
   {
      using Prop      = CtProp;
      using Property  = CtProperty;

      enum class Id
      {
         TableInitialize,    /// fired when a dataset is being loaded
         TableRemove,        /// fired when a dataset is being removed/detached.
         Sort,               /// fired when a dataset has been sorted
         Filter,             /// fired when a dataset has been filtered
         SubStringFilter,    /// fired when a substring filter has been applied to the dataset
         RowSelected,        /// fired when the user selects a row
         ColLayoutRequested  /// fired when user has requested list-view column auto-layout
      };

      Id                 event_id{};
      DatasetPtr         dataset{};
      NullableInt        affected_row{};
   };


} // namespace ctb