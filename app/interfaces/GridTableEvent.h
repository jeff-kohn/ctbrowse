/*******************************************************************
 * @file GridTableEvent.h
 *
 * @brief Header file defining GridTable events and related interfaces
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "interfaces/GridTable.h"

#include <memory>
#include <optional>


namespace ctb::app
{

   /// @brief categorizes the different notification events supported by this interface
   ///
   struct GridTableEvent
   {
      enum class Id
      {
         TableInitialize,  /// fired when a grid table is being loaded
         TableRemove,      /// fired when a grid table is being removed/detached.
         Sort,             /// fired when a grid table has been sorted
         Filter,           /// fired when a grid table has been filtered
         SubStringFilter,  /// fired when a substring filter has been applied to the grid table
         RowSelected       /// fired when the user selects a row
      };

      Id                 m_event_id{};
      GridTable*         m_grid_table{};
      std::optional<int> m_affected_row{};
   };

   /// @brief listener interface for classes that want to receive notification events about a grid table.
   struct IGridTableEventSink
   {

      /// @brief called to notify the sink that a table event has occurred.
      ///
      /// the supplied pointer will remain valid until a subsequent event
      /// notification of type TableInitialized is received
      /// 
      virtual void notify(GridTableEvent event) = 0;

  
      /// @brief virtual destructor
      ///
      virtual ~IGridTableEventSink()
      {}
   };


   /// @brief Interface for an event source that generates events for grid tables
   /// 
   struct IGridTableEventSource
   {
      /// @brief returns true if this source has a table attached, false otherwise
      ///
      virtual bool hasTable() const = 0;


      /// @brief retrieves a pointer to the active table for this source, if any.
      ///
      /// the returned table ptr may be null if this source doesn't have an active table.
      /// 
      virtual GridTablePtr getTable() = 0;


      /// @brief assigns a table to this source.
      /// 
      /// triggers the TableInitialize event IF a non-null table ptr is passed.
      /// 
      /// If a null table ptr is passed, this grid will no longer fire events 
      /// until a subsequent call to setTable() passes a valid pointer.
      /// 
      virtual bool setTable(GridTablePtr table) = 0;


      /// @brief attaches an event sink to this source to receive event notifications
      ///
      /// detach() must be called when notifications no longer can/should be sent to 
      /// the subscriber, otherwise there is no way for the source to determine validity of 
      /// attached subscribers.
      /// 
      virtual void attach(IGridTableEventSink* observer) = 0;


      /// @brief detach an event sink from this source to no longer receive event notifications
      ///
      virtual void detach(IGridTableEventSink* observer) = 0;


      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      virtual bool signal(GridTableEvent::Id event, std::optional<int> row_idx) = 0;


      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      virtual bool signal(GridTableEvent::Id event) = 0;


      /// @brief virtual destructor
      ///
      virtual ~IGridTableEventSource() noexcept
      {}
   };


   /// @brief smart ptr alias for shared ptr to IGridTableEventSource-derived
   /// 
   using GridTableEventSourcePtr = std::shared_ptr<IGridTableEventSource>;



}