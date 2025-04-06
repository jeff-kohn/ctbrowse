/*******************************************************************
 * @file GridTableEventSource.cpp
 *
 * @brief implementation file for the GridTableEventSource class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "grid/GridTableEventSource.h"


namespace ctb::app
{

   /// @brief static method to create a class instance.
   [[nodiscard]] GridTableEventSourcePtr GridTableEventSource::create()
   { 
      return GridTableEventSourcePtr{ new GridTableEventSource{} }; 
   }

   bool GridTableEventSource::hasTable() const
   { 
      return m_grid_table ? true : false; 
   }


   /// @brief assigns a table to this source.
   bool GridTableEventSource::setTable(GridTablePtr table)
   {
      SPDLOG_DEBUG("GridTableEventSource::setTable() called.");

      // We need to signal that the current table is being replaced, because
      // otherwise views that hold internal table pointers will be left with
      // dangling/garbage pointer
      if (!signal(GridTableEvent::Id::TableRemove))
         return false;

      m_grid_table = table;
      return signal(GridTableEvent::Id::TableInitialize);
   }


   /// @brief retrieves a pointer to the active table for this source, if any.
   GridTablePtr GridTableEventSource::getTable()
   {
      return m_grid_table;
   }


   /// @brief  attaches an event sink to this source to receive event notifications
   void GridTableEventSource::attach(IGridTableEventSink* observer) 
   {
      m_observers.insert(observer);
   }


   /// @brief detach an event sink from this source to no longer receive event notifications
   void GridTableEventSource::detach(IGridTableEventSink* observer)
   {
      auto it = m_observers.find(observer);
      if (it != m_observers.end()) 
         m_observers.erase(it);
   }


   /// @brief this is called to signal that an event needs to be sent to all listeners
   bool GridTableEventSource::signal(GridTableEvent::Id event_id, std::optional<int> row_idx) noexcept
   {
      SPDLOG_DEBUG("GridTableEventSource::signal({},{}) called", magic_enum::enum_name(event_id), row_idx.value_or(-1));

      bool retval{ true };

      if (m_grid_table)
      {
         for (auto observer : m_observers) 
         { 
            try
            {
               observer->notify({ event_id, m_grid_table.get(), row_idx }); 
            }
            catch(Error& err)
            {
               retval = false;
               wxGetApp().displayErrorMessage(err);
            }
            catch(std::exception& e)
            {
               retval = false;
               wxGetApp().displayErrorMessage(e.what());
            }
         }
      }

      return retval;
   }


   bool GridTableEventSource::signal(GridTableEvent::Id event_id) noexcept
   {
      return signal(event_id, std::nullopt);
   }


   GridTableEventSource::~GridTableEventSource() noexcept
   {
      // We can't guarantee that some event sink won't throw, so best to be safe.
      try
      {
         signal(GridTableEvent::Id::TableRemove);
      }
      catch(...)
      {
         try {
            auto e = packageError();
            log::error("~GridTableEventSource caught exception from signal(TableRemove) event: {}", e.what()); 
         }
         catch (...) {}
      } 
   }

} // namespace ctb::app