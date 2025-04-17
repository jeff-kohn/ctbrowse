/*******************************************************************
 * @file DatasetEventSource.cpp
 *
 * @brief implementation file for the DatasetEventSource class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "model/DatasetEventSource.h"


namespace ctb::app
{

   /// @brief static method to create a class instance.
   [[nodiscard]] DatasetEventSourcePtr DatasetEventSource::create()
   { 
      return DatasetEventSourcePtr{ new DatasetEventSource{} }; 
   }

   bool DatasetEventSource::hasTable() const
   { 
      return m_data ? true : false; 
   }


   /// @brief assigns a table to this source.
   bool DatasetEventSource::setTable(DatasetPtr table)
   {
      SPDLOG_DEBUG("DatasetEventSource::setTable() called.");

      // We need to signal that the current table is being replaced, because
      // otherwise views that hold internal table pointers will be left with
      // dangling/garbage pointer
      if (!signal(DatasetEvent::Id::TableRemove))
         return false;

      m_data = table;
      return signal(DatasetEvent::Id::TableInitialize);
   }


   /// @brief retrieves a pointer to the active table for this source, if any.
   DatasetPtr DatasetEventSource::getTable()
   {
      return m_data;
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
   bool DatasetEventSource::signal(DatasetEvent::Id event_id, std::optional<int> row_idx) noexcept
   {
      SPDLOG_DEBUG("DatasetEventSource::signal({},{}) called", magic_enum::enum_name(event_id), row_idx.value_or(-1));

      bool retval{ true };
      if (m_data)
      {
         for (auto observer : m_observers) 
         { 
            try
            {
               observer->notify({ event_id, m_data.get(), row_idx});
            }
            catch(...){
               retval = false;
               wxGetApp().displayErrorMessage(packageError(), true);
            }
         }
      }

      return retval;
   }


   bool DatasetEventSource::signal(DatasetEvent::Id event_id) noexcept
   {
      return signal(event_id, std::nullopt);
   }


   DatasetEventSource::~DatasetEventSource() noexcept
   {
      // We can't guarantee that some event sink won't throw, so best to be safe.
      try
      {
         signal(DatasetEvent::Id::TableRemove);
      }
      catch(...)
      {
         try {
            auto e = packageError();
            log::error("~DatasetEventSource caught exception from signal(TableRemove) event: {}", e.what()); 
         }
         catch (...) {}
      } 
   }

} // namespace ctb::app