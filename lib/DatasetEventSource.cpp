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


   auto DatasetEventSource::hasDataset() const  noexcept-> bool
   { 
      return m_data ? true : false; 
   }


   auto DatasetEventSource::getDataset() const  noexcept-> DatasetPtr
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


   void DatasetEventSource::attach(IDatasetEventSink* observer)  noexcept
   {
      SPDLOG_DEBUG("DatasetEventSource::attach() called.");
      m_observers.insert(observer);
   }


   void DatasetEventSource::detach(IDatasetEventSink* observer) noexcept
   {
      SPDLOG_DEBUG("DatasetEventSource::detach() called.");
      m_observers.erase(observer);
   }


   auto DatasetEventSource::signal(DatasetEvent::Id event_id, NullableInt rec_idx, IDatasetEventSink* event_source) noexcept -> bool
   {
      [[maybe_unused]] auto event_name = magic_enum::enum_name(event_id);
      SPDLOG_DEBUG("DatasetEventSource::signal({},{}) called", event_name, rec_idx.value_or(-1));

      bool retval{ true };
      if (m_data)
      {
         for (auto observer : m_observers) 
         { 
            try
            {
               if (observer != event_source)
               {
                  observer->notify({ event_id, m_data, rec_idx });
               }
            }
            catch(...){
               retval = false;
               SPDLOG_DEBUG(
                  "DatasetEventSource::signal({}, {}) caught exception from observer. {}", 
                  event_name, 
                  rec_idx.value_or(-1),
                  packageError().formattedMesage()
               );
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


   auto DatasetEventSource::signal(DatasetEvent::Id event, NullableInt rec_idx) noexcept -> bool 
   {
      return signal(event, rec_idx, nullptr);
   }


   DatasetEventSource::~DatasetEventSource() noexcept
   {
      signal(DatasetEvent::Id::DatasetRemove);
   }


} // namespace ctb