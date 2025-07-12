/*******************************************************************
 * @file ScopedEventSink.h
 *
 * @brief Header file for ScopedEventSink class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/interfaces/IDatasetEventSource.h"


namespace ctb
{
   /// @brief Scoped RAII wrapper for subscribing/unsubscribing from a dataset event source
   ///
   /// To handle dataset events, all a class needs to do is instantiate a member of this
   /// class, passing a pointer to the IDatasetEventSink interface and event source to its ctor. This class
   /// will automatically subscribe/unsubscribe to the event source in its ctor/dtor
   /// 
   class ScopedEventSink final
   {
   public:
      /// @brief construct a scoped event sink, attaching it to the specified source.
      ///
      /// @throw ctb::Error if sink or source is null
      ScopedEventSink(IDatasetEventSink* sink, DatasetEventSourcePtr source) noexcept(false) : m_sink{ sink }
      {
         if (!m_sink)
         {
            assert("sink ptr cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
         reset(std::move(source));
      }

      /// @brief attach this sink to the specified source
      /// @throw ctb::Error if source is null
      void reset(DatasetEventSourcePtr source) noexcept(false)
      {
         if (source)
         {
            detach();
            attach(std::move(source));
         }
         else {
            assert("source ptr cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
      }

      /// @brief method to signal the source (if we have one) to fire an event
      /// 
      /// If notify_self is false, the caller will receive a notification for this event. 
      /// If notify_self is false, the caller will not receive the event notification.
      ///
      /// @return true if successful, false if we don't have a source or the source couldn't
      ///  send all notifications
      auto signal_source(DatasetEvent::Id event_id, bool notify_self, NullableInt rec_idx = std::nullopt)
      {
         if (m_source)
         {
            return m_source->signal(event_id, rec_idx, notify_self ? nullptr : m_sink);
         }
         return false;
      }

      /// @brief returns the dataset currently associated with this source, if any
      /// 
      /// be sure to check the return value as it could be nullptr
      template<typename Self>
      [[nodiscard]] DatasetPtr getDataset(this Self&& self) noexcept
      {
         if (self.m_source)
         {
            return std::forward<Self>(self).m_source->getDataset();
         }
         return DatasetPtr{};
      }

      /// @brief returns the dataset currently associated with this source
      /// 
      /// be sure to check the return value as it could be nullptr
      template<typename Self>
      [[nodiscard]] DatasetPtr getDatasetOrThrow(this Self&& self) noexcept(false)
      {
         if (!self.hasDataset())
         {
            throw Error{ constants::ERROR_STR_NO_DATASET, Error::Category::DataError };
         }
         return std::forward<Self>(self).m_source->getDataset();
      }

      [[nodiscard]] auto getSource() const -> DatasetEventSourcePtr
      {
         return m_source;
      }

      /// @brief returns whether the event source has a dataset attached or not.
      bool hasDataset() const noexcept
      {
         return m_source->getDataset() != nullptr;
      }

      /// @brief destructor
      ~ScopedEventSink() noexcept
      {
         detach();
      }

   private:
      IDatasetEventSink*    m_sink{ nullptr };
      DatasetEventSourcePtr m_source{ nullptr };

      void attach(DatasetEventSourcePtr source) noexcept
      {
         if (source)
         {
            m_source = source;
            m_source->attach(m_sink);
         }
      }

      void detach() noexcept 
      {
         if (m_source)
         {
            m_source->detach(m_sink);
            m_source.reset();
         }
      }
   };


}  // namespace ctb