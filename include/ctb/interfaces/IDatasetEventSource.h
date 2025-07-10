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
      virtual auto hasDataset() const noexcept -> bool = 0;

      /// @brief Retrieves a (possibly nullptr) pointer to the active table for this source
      ///
      /// the returned dataset ptr may be null if this source doesn't have an active dataset.
      virtual auto getDataset() const noexcept -> DatasetPtr = 0;

      /// @brief assigns a dataset to this source.
      /// 
      /// Always triggers DatasetEvent::Id::DatasetRemove unless existing dataset == nullptr.
      /// Triggers DatasetEvent::Id::DatasetInitialize IF a non-null dataset ptr is passed 
      /// 
      /// If a null dataset ptr is passed, this view will no longer fire events 
      /// until a subsequent call to setDataset() passes a valid pointer.
      virtual void setDataset(DatasetPtr dataset) noexcept = 0;

      /// @brief assigns a dataset to this source.
      ///
      /// Always triggers DatasetEvent::Id::DatasetRemove unless existing dataset == nullptr.
      /// Triggers the DatasetEvent::Id::DatasetInitialize IF a non-null dataset ptr is passed 
      /// and signal_event == true
      ///
      /// If a null dataset ptr is passed, this view will no longer fire events 
      /// until a subsequent call to setDataset() passes a valid pointer.
      virtual void setDataset(DatasetPtr dataset, bool signal_event) noexcept = 0;

      /// @brief attaches an observer to this source to receive event notifications
      ///
      /// detach() must be called when notifications can/should no longer be sent to 
      /// the observer, because there is no way for this source to determine validity
      /// of the pointers-to-IDatasetEventSink it has.
      virtual void attach(IDatasetEventSink* observer) noexcept = 0;

      /// @brief detach an observer from this source to no longer receive event notifications
      ///
      /// This must be called when notifications can/should no longer be sent to 
      /// an observer, because there is no way for this source to determine validity
      /// of the pointers-to-IDatasetEventSink it has.
      virtual void detach(IDatasetEventSink* observer) noexcept = 0;

      /// @brief this is called to signal that an event needs to be sent to all observers
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      virtual bool signal(DatasetEvent::Id event) noexcept = 0;

      /// @brief this is called to signal that an event needs to be sent to all observers EXCEPT 
      ///  for event_source. 
      ///
      /// This allows a caller to avoid receiving self-generated events if necessary/preferable
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      virtual auto signal(DatasetEvent::Id event, IDatasetEventSink* event_source) noexcept -> bool = 0;

      /// @brief this is called to signal that an event needs to be sent to all observers
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      virtual auto signal(DatasetEvent::Id event, NullableInt rec_idx) noexcept -> bool = 0;

      /// @brief this is called to signal that an event needs to be sent to all observers EXCEPT 
      ///  for event_source. 
      ///
      /// This allows a caller to avoid receiving self-generated events if necessary/preferable
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      virtual auto signal(DatasetEvent::Id event, NullableInt rec_idx, IDatasetEventSink* event_source) noexcept -> bool = 0;

      /// @brief virtual destructor
      virtual ~IDatasetEventSource() noexcept
      {}
   };

   using DatasetEventSourcePtr = std::shared_ptr<IDatasetEventSource>;

} // namespace ctb