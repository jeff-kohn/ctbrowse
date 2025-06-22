/*******************************************************************
 * @file ScopedEventSink.h
 *
 * @brief Header file for ScopedEventSink class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/model/DatasetEventSource.h"


namespace ctb
{
   /// @brief Scoped RAII wrapper for subscribing/unsubscribing from a dataset event source
   ///
   /// To handle dataset events, all a class needs to do is instantiate a member of this
   /// class, passing a pointer to the IDatasetEventSink interface to its ctor. This class
   /// will automatically subscribe/unsubscribe to the event source in its ctor/dtor
   /// 
   class ScopedEventSink final
   {
   public:
      /// @brief construct a scoped event sink without attaching it to a source
      explicit ScopedEventSink(IDatasetEventSink* sink) noexcept(false) : m_sink{sink}
      {
         if (!m_sink)
         {
            assert("sink ptr cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
      }

      /// @brief construct a scoped event sink, attaching it to the specified source.
      ///
      /// you can pass a null source (although there's no point), but passing a null 
      /// sink will throw an exception.
      ScopedEventSink(IDatasetEventSink* sink, DatasetEventSourcePtr source) noexcept(false) : m_sink{sink}, m_source{source}
      {
         if (!m_sink)
         {
            assert("sink ptr cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
         attach();
      }

      /// @brief attach this sink to the specified source
      void reset(DatasetEventSourcePtr source)
      {
         detach();
         m_source = source;
         attach();
      }

      /// @brief method to signal the source (if we have one) to fire an event
      ///
      /// returns true if successful, false if we don't have a source or the source 
      /// couldn't send notifications (because of no current dataset, for instance)
      bool signal_source(DatasetEvent::Id event_id, NullableInt rec_idx = std::nullopt)
      {
         if (m_source)
         {
            return m_source->signal(event_id, rec_idx);
         }
         return false;
      }

      /// @brief returns the dataset currently associated with this source, if any
      /// 
      /// be sure to check the return value as it could be nullptr
      template<typename Self>
      [[nodiscard]] DatasetPtr getDataset(this Self&& self)
      {
         if (self.m_source)
         {
            return std::forward<Self>(self).m_source->getDataset();
         }
         return DatasetPtr{};
      }

      /// @brief returns whether the event source has a dataset attached or not.
      bool hasDataset() const
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

      void attach() noexcept
      {
         if (m_source)
         {
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