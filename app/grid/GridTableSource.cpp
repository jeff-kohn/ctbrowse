/*******************************************************************
 * @file GridTableSource.cpp
 *
 * @brief implementation file for the GridTableSource class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "grid/GridTableSource.h"


namespace ctb::app
{

   /// @brief static method to create a class instance.
   [[nodiscard]] GridTableEventSourcePtr GridTableSource::create()
   { 
      return GridTableEventSourcePtr{ new GridTableSource{} }; 
   }

   bool GridTableSource::hasTable() const
   { 
      return m_table ? true : false; 
   }


   /// @brief assigns a table to this source.
   void GridTableSource::setTable(GridTablePtr table)
   {
      m_table = table;
      signal(GridTableEvent::TableInitialize);
   }


   /// @brief retrieves a pointer to the active table for this source, if any.
   GridTablePtr GridTableSource::getTable()
   {
      return m_table;
   }


   /// @brief  attaches an event sink to this source to receive event notifications
   void GridTableSource::attach(IGridTableEventSink* observer) 
   {
      m_observers.insert(observer);
   }

   /// @brief detach an event sink from this source to no longer receive event notifications
   void GridTableSource::detach(IGridTableEventSink* observer)
   {
      auto it = m_observers.find(observer);
      if (it != m_observers.end()) 
         m_observers.erase(it);
   }

   /// @brief this is called to signal that an event needs to be sent to all listeners
   bool GridTableSource::signal(GridTableEvent event)
   {
      if (m_table)
      {
         for (auto observer : m_observers) { observer->notify(event, m_table.get()); }
         return true;
      }
      return false;
   }
} // namespace ctb::app