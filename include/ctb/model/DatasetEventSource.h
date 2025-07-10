/*******************************************************************
 * @file DatasetEventSource.h
 *
 * @brief Header file for DatasetEventSource class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/interfaces/IDatasetEventSource.h"

#include <memory>
#include <unordered_set>


namespace ctb
{

   /// @brief a single-threaded default implementation of IDatasetEventSource interface
   ///
   /// this implementation is not thread-safe, since we're using it with UI classes that must
   /// only be accessed from the main thread. If communication with background threads is
   /// needed, manual synchronization or a different implementation will be necessary.
   /// 
   class DatasetEventSource : public IDatasetEventSource
   {
   public:
      /// @brief static method to create a class instance.
      ///
      /// note that while you can attach/detach from this object immediately, 
      /// getDataset() will return nullptr and the object won't fire any events 
      /// until a valid table ptr is passed to setDataset().
      [[nodiscard]] static auto create() -> DatasetEventSourcePtr;

      /// @brief returns true if this source has a table attached, false otherwise
      ///
      auto hasDataset() const -> bool override;

      /// @brief retrieves a pointer to the active table for this source, if any.
      ///
      /// the returned table ptr may be null if this source doesn't have an active table.
      /// 
      auto getDataset() const -> DatasetPtr override;

      /// @brief assigns a table to this source.
      /// 
      /// triggers the DatasetRemove event before disconnecting the current table (if it is non-null)
      /// triggers the DatasetInitialize event for the new table-ptr (if it is non-null and signal_event
      /// is true)
      /// 
      /// If a null table ptr is passed, this source will no longer fire events 
      /// until a subsequent call to setDataset() passes a valid pointer.
      /// 
      auto setDataset(DatasetPtr dataset, bool signal_event) -> bool override;
      
      /// @brief assigns a table to this source.
      /// 
      /// Triggers the DatasetRemove event before disconnecting the current table (if it is non-null)
      /// Triggers the DatasetInitialize event for the new table-ptr (if it is non-null)
      /// 
      /// If a null table ptr is passed, this source will no longer fire events 
      /// until a subsequent call to setDataset() passes a valid pointer.
      /// 
      void setDataset(DatasetPtr dataset) override
      {
         setDataset(dataset, true);
      }

      /// @brief attaches an event sink to this source to receive event notifications
      ///
      /// detach() must be called when notifications no longer can/should be sent to 
      /// the subscriber, otherwise there is no way for the source to determine validity of 
      /// attached subscribers.
      /// 
      void attach(IDatasetEventSink* observer) override;

      /// @brief detach an event sink from this source to no longer receive event notifications
      void detach(IDatasetEventSink* observer) override;

      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      /// sinks should try to handle their own exceptions if it's possible to do so gracefully;
      /// exceptions caught in this function will be logged in debug builds but otherwise lost
      /// 
      /// @return true if every subscriber was notified without error, false if at least one
      ///         subscriber threw an error.
      auto signal(DatasetEvent::Id event, NullableInt rec_idx) noexcept -> bool override;
 
      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      /// sinks should try to handle their own exceptions if it's possible to do so gracefully. Any 
      /// exceptions propagated back to this function will be displayed to the user.
      /// 
      /// @return true if every subscriber was notified without error, false if at least one
      ///         subscriber threw an error.
      auto signal(DatasetEvent::Id event_id) noexcept -> bool override;

      /// @brief destructor
      ~DatasetEventSource() noexcept override;

   private:
      DatasetPtr m_data{};
      std::unordered_set<IDatasetEventSink*> m_observers{};
   
      /// @brief default ctor is private, use static create()
      DatasetEventSource() = default;

      // no copy/move/assign, this class is created on the heap and passed around in shared_ptr
      DatasetEventSource(const DatasetEventSource&) = delete;
      DatasetEventSource(DatasetEventSource&&) = delete;
      DatasetEventSource& operator=(const DatasetEventSource&) = delete;
      DatasetEventSource& operator=(DatasetEventSource&&) = delete;   
   };



}  // namespace  ctb