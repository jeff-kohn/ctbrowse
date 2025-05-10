/*******************************************************************
* @file IDatasetEventSource.h
*
* @brief Header file defining IDatasetEventSource interface.
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/interfaces/IDatasetEventSink.h"


namespace ctb
{
   /// @brief Interface for an event source that generates events for datasets
   /// 
   struct IDatasetEventSource
   {
      /// @brief returns true if this source has a dataset attached, false otherwise
      ///
      virtual auto hasTable() const -> bool = 0;

      /// @brief retrieves a pointer to the active dataset for this source, if any.
      ///
      /// the returned dataset ptr may be null if this source doesn't have an active dataset.
      /// 
      virtual auto getTable() const -> DatasetPtr = 0;

      /// @brief assigns a dataset to this source.
      /// 
      /// triggers the TableInitialize event IF a non-null dataset ptr is passed.
      /// 
      /// If a null dataset ptr is passed, this view will no longer fire events 
      /// until a subsequent call to setTable() passes a valid pointer.
      /// 
      virtual void setTable(DatasetPtr dataset) = 0;

      /// @brief assigns a dataset to this source.
      /// 
      /// triggers the TableInitialize event IF a non-null dataset ptr is passed.
      /// 
      /// If a null dataset ptr is passed, this view will no longer fire events 
      /// until a subsequent call to setTable() passes a valid pointer.
      /// 
      /// @return true if non-null dataset was passed and all sinks were successfully
      ///         notified, false otherwise.
      /// 
      virtual auto setTable(DatasetPtr dataset, bool signal_event) -> bool = 0;

      /// @brief attaches an event sink to this source to receive event notifications
      ///
      /// detach() must be called when notifications no longer can/should be sent to 
      /// the subscriber, otherwise there is no way for the source to determine validity of 
      /// attached subscribers.
      /// 
      virtual void attach(IDatasetEventSink* observer) = 0;

      /// @brief detach an event sink from this source to no longer receive event notifications
      ///
      virtual void detach(IDatasetEventSink* observer) = 0;

      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      virtual auto signal(DatasetEvent::Id event, NullableInt rec_idx) -> bool = 0;

      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      virtual bool signal(DatasetEvent::Id event) = 0;

      /// @brief virtual destructor
      ///
      virtual ~IDatasetEventSource() noexcept
      {}
   };

} // namespace ctb