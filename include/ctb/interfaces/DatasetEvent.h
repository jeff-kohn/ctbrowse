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
   /// @brief Event object passed for Dataset-related events 
   ///
   struct DatasetEvent
   {
      using Prop        = CtProp;
      using PropertyVal = CtPropertyVal;

      enum class Id
      {
         DatasetInitialize,  /// fired when a dataset is being loaded
         DatasetRemove,      /// fired when a dataset is being removed/detached.
         Sort,               /// fired when a dataset has been sorted
         Filter,             /// fired when a dataset has been filtered
         SubStringFilter,    /// fired when a substring filter has been applied to the dataset
         RowSelected,        /// fired when the user selects a row
      };

      /// @brief Identifier for the type of event this object represents.
      Id event_id{};

      /// @brief Pointer to active dataset. This will never be null unless event_id == Id::DatasetRemove
      DatasetPtr dataset{};

      /// @brief The zero-based index of the dataset row that generated the event. Will be null/empty for 
      ///  dataset-level events.
      NullableInt affected_row{};
   };


} // namespace ctb