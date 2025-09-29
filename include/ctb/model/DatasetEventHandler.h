/*******************************************************************
 * @file DatasetEventHandler.h
 *
 * @brief Header file for DatasetEventHandler class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/interfaces/IDatasetEventSource.h"
#include <memory>
#include <unordered_map>

namespace ctb
{


   /// @brief Scoped RAII wrapper for subscribing/unsubscribing event handlers for a datasource
   ///
   /// This class is meant to be used as a member in another class that wants to handle DatasetEvents. 
   /// 
   /// The functors passed to addHandler() must be valid for the lifetime of this object, including any 
   /// captured pointers.
   /// 
   /// This class is not thread-safe at the instance level, because you could get a race condition where
   /// the notify callback gets called during or just after destruction.
   /// 
   class DatasetEventHandler final : public IDatasetEventSink
   {
   public:
      using WeakRef                = std::weak_ptr<DatasetEventHandler>;
      using EventId                = DatasetEvent::Id;
      using EventCallback          = std::function<void(DatasetEvent& event)>;
      using CallbackMap            = std::unordered_map<EventId, EventCallback>;


      DatasetEventHandler(DatasetEventSourcePtr source) noexcept : m_source(std::move(source))
      {
         if (!m_source)
         {
            assert("source ptr cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };            
         }
         m_source->attach(this);
      }


      /// @brief returns whether the attached event source has a dataset attached or not.
      bool hasDataset() const noexcept
      {
         return m_source->getDataset() != nullptr;
      }
      

      /// @brief returns a copy of the dataset currently associated with this source, if any
      /// 
      /// @param throw_if_null - if true, throw an exception on nullptr dataset; otherwise return nullptr;
      [[nodiscard]] auto getDataset(bool throw_if_null = true) noexcept(false) -> DatasetPtr 
      {
         if (hasDataset() == false and throw_if_null)
         {
            throw Error{ constants::ERROR_STR_NO_DATASET, Error::Category::DataError };
         }
         
         return m_source->getDataset();
      }


      /// @brief Returns a copy of the event source ptr this object is subscribed to
      [[nodiscard]] auto getSource() -> DatasetEventSourcePtr
      {
         return m_source;
      }


      /// @brief Add a handler for the specified event type.
      /// 
      /// Any existing handler for event_id will be replaced. 
      /// 
      /// The supplied callback must remain valid for the lifetime of this object or until you call 
      /// removeHandler() for the event, so be careful with callables containing captured pointers.
      /// 
      void addHandler(EventId event_id, EventCallback callback)
      {
         m_callbacks[event_id] = std::move(callback);
      }


      /// @brief Unsubscribe from notifications for the specified event_id
      /// @param event_id 
      void removeHandler(EventId event_id)
      {
         m_callbacks.erase(event_id);
      }


      /// @brief method to signal the source to fire an event
      /// 
      /// If notify_self is true,  the caller will receive a notification for this event.  
      /// If notify_self is false, the caller will NOT receive notification for this event.
      ///
      /// @return true if successful, false if the source couldn'tsend all notifications (meaning
      ///  at least one notfication threw an error)
      auto signal_source(DatasetEvent::Id event_id, bool notify_self, NullableInt rec_idx = std::nullopt) noexcept -> bool
      {
          return m_source->signal(event_id, rec_idx, notify_self ? nullptr : this);
      }


      /// @brief destructor
      ~DatasetEventHandler() noexcept override
      {
         detach();
      }

      DatasetEventHandler() = delete;
      DatasetEventHandler(const DatasetEventHandler&) = default;
      DatasetEventHandler(DatasetEventHandler&&) = default;
      DatasetEventHandler& operator=(const DatasetEventHandler&) = default;
      DatasetEventHandler& operator=(DatasetEventHandler&&) = default;

   private:
      DatasetEventSourcePtr m_source{ nullptr };
      CallbackMap           m_callbacks{};

      void notify(DatasetEvent event) override
      {
         auto it = m_callbacks.find(event.event_id);
         if (it != m_callbacks.end())
         {
            it->second(event);
         }
      }

      void detach() noexcept 
      {
         if (m_source)
         {
            m_source->detach(this);
            m_source.reset();
         }
      }
   };

}  // namespace ctb