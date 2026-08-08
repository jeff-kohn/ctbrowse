/*******************************************************************
 * @file DatasetEventSource.cpp
 *
 * @brief implementation file for the DatasetEventSource class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "ctb/model/DatasetEventSource.h"


namespace ctb
{

   /// @brief static method to create a class instance.
   [[nodiscard]]
   auto DatasetEventSource::create() -> DatasetEventSourcePtr
   {
      return DatasetEventSourcePtr{ new DatasetEventSource{} };
   }


   auto DatasetEventSource::hasDataset() const noexcept -> bool
   {
      return m_data != nullptr;
   }


   auto DatasetEventSource::getDataset() const noexcept -> DatasetPtr
   {
      return m_data;
   }


   void DatasetEventSource::setDataset(DatasetPtr dataset, bool signal_event) noexcept
   {
      SPDLOG_DEBUG("DatasetEventSource::setDataset() called.");

      // We need to signal that the current dataset is being replaced, because
      // views may contain state that is invalidated by the change.
      signal(DatasetEvent::Id::DatasetRemove);

      m_data = dataset;
      if (signal_event)
      {
         signal(DatasetEvent::Id::DatasetInitialize);
      }
   }


   void DatasetEventSource::attach(IDatasetEventSink* observer) noexcept
   {
      SPDLOG_TRACE("DatasetEventSource::attach() called.");
      m_observers.insert(observer);
   }


   void DatasetEventSource::detach(IDatasetEventSink* observer) noexcept
   {
      SPDLOG_TRACE("DatasetEventSource::detach() called.");
      m_observers.erase(observer);
   }


   auto DatasetEventSource::signal(DatasetEvent::Id event_id, NullableUInt rec_idx, IDatasetEventSink* event_source) noexcept -> bool
   {
      [[maybe_unused]] auto event_name = enum_to_string(event_id);
      SPDLOG_TRACE("DatasetEventSource::signal({},{}) called", event_name, static_cast<int>(rec_idx.value_or(-1)));

      bool retval{ true };
      if (m_data)
      {
         if (rec_idx) m_data->moveToRow(*rec_idx);

         for (auto* observer : m_observers)
         {
            try
            {
               if (observer != event_source)
               {
                  observer->notify({ event_id, m_data.get(), rec_idx });
               }
            }
            catch (...)
            {
               retval = false;
               SPDLOG_DEBUG("DatasetEventSource::signal({}, {}) caught exception from observer. {}",
                            event_name,
                            rec_idx.value_or(-1),
                            packageError().formattedMessage());
            }
         }
      }
      return retval;
   }


   auto DatasetEventSource::signal(DatasetEvent::Id event) noexcept -> bool
   {
      return signal(event, std::nullopt, nullptr);
   }


   auto DatasetEventSource::signal(DatasetEvent::Id event, IDatasetEventSink* event_source) noexcept -> bool
   {
      return signal(event, std::nullopt, event_source);
   }


   auto DatasetEventSource::signal(DatasetEvent::Id event, NullableUInt rec_idx) noexcept -> bool
   {
      return signal(event, rec_idx, nullptr);
   }

}   // namespace ctb
