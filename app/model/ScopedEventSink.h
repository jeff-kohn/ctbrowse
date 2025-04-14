/*******************************************************************
 * @file ScopedEventSink.h
 *
 * @brief Header file for ScopedEventSink class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/DatasetEvent.h"


namespace ctb::app
{
   /// @brief scoped RAII wrapper for subscribing/unsubscribing from a grid table event source
   ///
   /// to handle grid-table events, all a class needs to do is instantiate a member of this
   /// class, passing a pointer to the IDatasetEventSink interface to its ctor (usually 'this'
   /// when the containing class itself inherits from IDatasetEventSink to ensure that the sink
   /// interface is available for the lifetime of the event source).
   class ScopedEventSink final
   {
   public:
      /// @brief construct a scoped event sink without attaching it to a source
      ///
      explicit ScopedEventSink(IDatasetEventSink* sink) : m_sink{ sink }
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
      /// 
      ScopedEventSink(IDatasetEventSink* sink, DatasetEventSourcePtr source) : 
         m_sink{ sink },
         m_source{ source }
      {
         if (!m_sink)
         {
            assert("sink ptr cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
         attach();
      }

      /// @brief attach this sink to the specified source
      ///
      void reset(DatasetEventSourcePtr source)
      {
         detach();
         m_source = source;
         attach();
      }

      /// @brief method to signal the source (if we have one) to fire an event
      ///
      /// returns true if successful, false if we don't have a source or the source 
      /// couldn't send notifications (because of no current table, for instance)
      /// 
      bool signal_source(DatasetEvent::Id event_id, std::optional<int> row_idx = std::nullopt)
      {
         if (m_source)
         {
            return m_source->signal(event_id, row_idx);
         }
         return false;
      }

      /// @brief returns the table currently associated with this source, if any
      /// 
      /// be sure to check the return value as it could be nullptr
      /// 
      [[nodiscard]] IDatasetPtr getTable()
      {
         if (m_source)
         {
            return m_source->getTable();
         }
         return IDatasetPtr{};
      }

      /// @brief returns whether the event source has a table attached or not.
      /// 
      bool hasTable() const
      {
         return m_source->getTable() != nullptr;
      }

      /// @brief destructor
      ///
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


}  // namespace ctb::app