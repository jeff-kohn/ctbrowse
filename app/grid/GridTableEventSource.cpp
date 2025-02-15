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
      // We need to signal that the current table is being replaced, because
      // otherwise views that hold internal table pointers will be left with
      // dangling/garbage pointer
      if (! signal(GridTableEvent::TableRemove) )
         return false;

      m_grid_table = table;
      return signal(GridTableEvent::TableInitialize);
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
   bool GridTableEventSource::signal(GridTableEvent event) noexcept
   {
      bool retval{ true };

      if (m_grid_table)
      {
         for (auto observer : m_observers) 
         { 
            try
            {
               observer->notify(event, m_grid_table.get()); 
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


   GridTableEventSource::~GridTableEventSource() noexcept
   {
      // We can't guarantee that some event sink won't throw, so best to be safe.
      try
      {
         signal(GridTableEvent::TableRemove);
      }
      catch(...){} // TODO: logging to OutputDebugString maybe? not much we can safely do from here.
   }

} // namespace ctb::app