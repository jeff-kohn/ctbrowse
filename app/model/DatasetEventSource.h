/*******************************************************************
 * @file DatasetEventSource.h
 *
 * @brief Header file for DatasetEventSource class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/DatasetEvent.h"

#include <memory>
#include <unordered_set>


namespace ctb::app
{

   /// @brief a single-threaded default implementation of IDatasetEventSource interface
   ///
   /// this implementation is not thread-safe, since we're using it with UI classes that must
   /// only be accessed from the main thread. If communication with background threads is
   /// needed, a different implementation will be necessary.
   /// 
   class DatasetEventSource final : public IDatasetEventSource
   {
   public:      
      /// @brief static method to create a class instance.
      ///
      /// note that while you can attach/detach from this object immediately, 
      /// getTable() will return nullptr and the object won't fire any events 
      /// until a valid table ptr is passed to setTable().
      /// 
      [[nodiscard]] static DatasetEventSourcePtr create();

      /// @brief returns true if this source has a table attached, false otherwise
      ///
      bool hasTable() const  override;


      /// @brief retrieves a pointer to the active table for this source, if any.
      ///
      /// the returned table ptr may be null if this source doesn't have an active table.
      /// 
      IDatasetPtr getTable() override;

      /// @brief assigns a table to this source.
      /// 
      /// triggers the TableRemove event before disconnecting the current table (if it is non-null)
      /// triggers the TableInitialize event for the new table-ptr (if it is non-null)
      /// 
      /// If a null table ptr is passed, this grid will no longer fire events 
      /// until a subsequent call to setTable() passes a valid pointer.
      /// 
      bool setTable(IDatasetPtr table) override;

      /// @brief attaches an event sink to this source to receive event notifications
      ///
      /// detach() must be called when notifications no longer can/should be sent to 
      /// the subscriber, otherwise there is no way for the source to determine validity of 
      /// attached subscribers.
      /// 
      void attach(IDatasetEventSink* observer) override;

      /// @brief detach an event sink from this source to no longer receive event notifications
      /// 
      void detach(IDatasetEventSink* observer) override;

      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      /// sinks should try to handle their own exceptions if it's possible to do so gracefully. Any 
      /// exceptions propagated back to this function will be displayed to the user.
      /// 
      /// @return true if every subscriber was notified without error, false if at least one
      ///         subscriber threw an error.
      /// 
      bool signal(DatasetEvent::Id event_id, std::optional<int> row_idx) noexcept override;
 
      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      /// sinks should try to handle their own exceptions if it's possible to do so gracefully. Any 
      /// exceptions propagated back to this function will be displayed to the user.
      /// 
      /// @return true if every subscriber was notified without error, false if at least one
      ///         subscriber threw an error.
      /// 
      bool signal(DatasetEvent::Id event_id) noexcept override;

      /// @brief destructor
      ~DatasetEventSource() noexcept override;

   private:
      IDatasetPtr m_grid_table{};
      std::unordered_set<IDatasetEventSink*> m_observers{};
   
      /// @brief default ctor is private, use static create()
      DatasetEventSource() = default;

      // no copy/move/assign, this class is created on the heap.
      DatasetEventSource(const DatasetEventSource&) = delete;
      DatasetEventSource(DatasetEventSource&&) = delete;
      DatasetEventSource& operator=(const DatasetEventSource&) = delete;
      DatasetEventSource& operator=(DatasetEventSource&&) = delete;   
   };


}  // namespace  ctb::app