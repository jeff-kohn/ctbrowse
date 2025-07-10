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

   /// @brief A single-threaded default implementation of IDatasetEventSource interface
   ///
   /// Sinks should handle their own exceptions if possible; any exceptions caught by this 
   /// class while sending notifications will be logged in debug builds but otherwise lost.
   ///
   /// This implementation is not thread-safe, since we're using it with UI classes that must
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
      auto hasDataset() const noexcept -> bool override;

      /// @brief Retrieves a (possibly nullptr) pointer to the active table for this source
      ///
      /// the returned table ptr may be null if this source doesn't have an active dataset.
      auto getDataset() const noexcept -> DatasetPtr override;

      /// @brief assigns a table to this source.
      /// 
      /// Always triggers DatasetEvent::Id::DatasetRemove unless existing dataset == nullptr.
      /// Triggers DatasetEvent::Id::DatasetInitialize IF a non-null dataset ptr is passed 
      /// 
      /// If a null table ptr is passed, this source will no longer fire events 
      /// until a subsequent call to setDataset() passes a valid pointer.
      /// 
      void setDataset(DatasetPtr dataset) noexcept override
      {
         setDataset(dataset, true);
      }

      /// @brief assigns a dataset to this source.
      ///
      /// Always triggers DatasetEvent::Id::DatasetRemove unless existing dataset == nullptr.
      /// Triggers DatasetEvent::Id::DatasetInitialize IF a non-null dataset ptr is passed 
      /// and signal_event == true
      ///
      /// If a null dataset ptr is passed, this view will no longer fire events 
      /// until a subsequent call to setDataset() passes a valid pointer.
      void setDataset(DatasetPtr dataset, bool signal_event) noexcept override;

      /// @brief attaches an observer to this source to receive event notifications
      ///
      /// detach() must be called when notifications can/should no longer be sent to 
      /// the observer, because there is no way for this source to determine validity
      /// of the pointers-to-IDatasetEventSink it has.
      void attach(IDatasetEventSink* observer) noexcept override;

      /// @brief detach an observer from this source to no longer receive event notifications
      ///
      /// This must be called when notifications can/should no longer be sent to 
      /// an observer, because there is no way for this source to determine validity
      /// of the pointers-to-IDatasetEventSink it has.
      void detach(IDatasetEventSink* observer) noexcept override;

      /// @brief this is called to signal that an event needs to be sent to all observers
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      auto signal(DatasetEvent::Id event_id) noexcept -> bool override;

      /// @brief this is called to signal that an event needs to be sent to all observers EXCEPT 
      ///  for event_source. 
      ///
      /// This allows a caller to avoid receiving self-generated events if necessary/preferable
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      auto signal(DatasetEvent::Id event, IDatasetEventSink* event_source) noexcept -> bool override;

      /// @brief this is called to signal that an event needs to be sent to all observers
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      auto signal(DatasetEvent::Id event, NullableInt rec_idx) noexcept -> bool override;
 
      /// @brief this is called to signal that an event needs to be sent to all observers EXCEPT 
      ///  for event_source. 
      ///
      /// This allows a caller to avoid receiving self-generated events if necessary/preferable
      /// 
      /// @return true if every observer was notified without error, false if at least one
      ///  observer threw an error.
      auto signal(DatasetEvent::Id event, NullableInt rec_idx, IDatasetEventSink* event_source) noexcept -> bool override;

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