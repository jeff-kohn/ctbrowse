/*******************************************************************
 * @file DatasetEvent.h
 *
 * @brief Header file defining IDataset event class and related 
          source/sink interfaces.
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/DatasetBase.h"

#include <memory>
#include <optional>


namespace ctb::app
{
   /// @brief categorizes the different notification events supported by this interface
   ///
   struct DatasetEvent
   {
      enum class Id
      {
         TableInitialize,    /// fired when a dataset is being loaded
         TableRemove,        /// fired when a dataset is being removed/detached.
         Sort,               /// fired when a dataset has been sorted
         Filter,             /// fired when a dataset has been filtered
         SubStringFilter,    /// fired when a substring filter has been applied to the dataset
         RowSelected,        /// fired when the user selects a row
         ColLayoutRequested /// fired when user has requested listview column auto-layout
      };

      Id                 m_event_id{};
      IDataset*          m_data{};
      std::optional<int> m_affected_row{};
   };


   /// @brief listener interface for classes that want to receive notification events about a dataset.
   ///
   struct IDatasetEventSink
   {
      /// @brief called to notify the sink that a dataset event has occurred.
      ///
      /// The supplied pointer will remain valid until a subsequent event
      /// notification of type TableInitialize is received.
      /// 
      /// Event is passed by value because it can't be const-ref and we want to be able to pass
      /// temporaries. Also makes clear that changes to the event itself don't
      /// propagate back to caller.
      /// 
      virtual void notify(DatasetEvent event) = 0;
      

      /// @brief virtual destructor
      ///
      virtual ~IDatasetEventSink() = default;
   };


   /// @brief Interface for an event source that generates events for datasets
   /// 
   struct IDatasetEventSource
   {
      /// @brief returns true if this source has a table attached, false otherwise
      ///
      virtual bool hasTable() const = 0;


      /// @brief retrieves a pointer to the active table for this source, if any.
      ///
      /// the returned table ptr may be null if this source doesn't have an active table.
      /// 
      virtual IDatasetPtr getTable() = 0;


      /// @brief assigns a table to this source.
      /// 
      /// triggers the TableInitialize event IF a non-null table ptr is passed.
      /// 
      /// If a null table ptr is passed, this view will no longer fire events 
      /// until a subsequent call to setTable() passes a valid pointer.
      /// 
      virtual bool setTable(IDatasetPtr table) = 0;


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
      virtual bool signal(DatasetEvent::Id event, std::optional<int> row_idx) = 0;


      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      virtual bool signal(DatasetEvent::Id event) = 0;


      /// @brief virtual destructor
      ///
      virtual ~IDatasetEventSource() noexcept
      {}
   };


   /// @brief smart ptr alias for shared ptr to IDatasetEventSource-derived
   /// 
   using DatasetEventSourcePtr = std::shared_ptr<IDatasetEventSource>;



}