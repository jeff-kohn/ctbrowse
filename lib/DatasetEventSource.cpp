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
   auto DatasetEventSource::create() -> DatasetEventSource::DatasetEventSourcePtr
   { 
      return DatasetEventSourcePtr{ new DatasetEventSource{} }; 
   }

   auto DatasetEventSource::hasDataset() const -> bool
   { 
      return m_data ? true : false; 
   }


   /// @brief retrieves a pointer to the active table for this source, if any.
   auto DatasetEventSource::getDataset() const -> DatasetPtr
   {
      return m_data;
   }


   /// @brief assigns a table to this source.
   auto DatasetEventSource::setDataset(DatasetPtr dataset, bool signal_event) -> bool
   {
      SPDLOG_DEBUG("DatasetEventSource::setDataset() called.");

      // We need to signal that the current table is being replaced, because
      // otherwise views that hold internal table pointers will be left with
      // obsolete
      if (!signal(DatasetEvent::Id::DatasetRemove))
         return false;

      m_data = dataset;
      return signal_event ? signal(DatasetEvent::Id::DatasetInitialize) : true;
   }


   /// @brief  attaches an event sink to this source to receive event notifications
   void DatasetEventSource::attach(IDatasetEventSink* observer) 
   {
      SPDLOG_DEBUG("DatasetEventSource::attach() called.");
      m_observers.insert(observer);
   }


   /// @brief detach an event sink from this source to no longer receive event notifications
   void DatasetEventSource::detach(IDatasetEventSink* observer)
   {
      SPDLOG_DEBUG("DatasetEventSource::detach() called.");
      auto it = m_observers.find(observer);
      if (it != m_observers.end()) 
         m_observers.erase(it);
   }


   /// @brief this is called to signal that an event needs to be sent to all listeners
   auto DatasetEventSource::signal(DatasetEvent::Id event_id, NullableInt rec_idx) noexcept -> bool
   {
      auto event_name = magic_enum::enum_name(event_id);
      SPDLOG_DEBUG("DatasetEventSource::signal({},{}) called", event_name, rec_idx.value_or(-1));

      bool retval{ true };
      if (m_data)
      {
         for (auto observer : m_observers) 
         { 
            try
            {
               observer->notify({ event_id, m_data, rec_idx});
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


   auto DatasetEventSource::signal(DatasetEvent::Id event_id) noexcept -> bool
   {
      return signal(event_id, std::nullopt);
   }


   DatasetEventSource::~DatasetEventSource() noexcept
   {
      // We can't guarantee that some event sink won't throw, so best to be safe.
      try
      {
         signal(DatasetEvent::Id::DatasetRemove);
      }
      catch(...)
      {
         try {
            auto e = packageError();
            log::error("~DatasetEventSource caught exception from signal(DatasetRemove) event: {}", e.what()); 
         }
         catch (...) {}
      } 
   }

} // namespace ctb