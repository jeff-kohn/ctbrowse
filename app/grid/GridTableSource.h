
#pragma once

#include "interfaces/GridTableEvent.h"

#include <memory>
#include <unordered_set>

namespace ctb::app
{

   /// @brief a single-threaded default implementation of IGridTableEventSource interface
   ///
   /// this implementation is not thread-safe, since we're using it with UI classes that must
   /// only be accessed from the main thread. If communication with background threads is
   /// needed, a different implementation will be necessary.
   /// 
   class GridTableSource final : public IGridTableEventSource
   {
   public:
      
      /// @brief static method to create a class instance.
      ///
      /// note that while you can attach/detach from this object immediately, 
      /// getTable() will return nullptr and the object won't fire any events 
      /// until a valid table ptr is passed to setTable().
      /// 
      static [[nodiscard]] GridTableEventSourcePtr create();


      /// @brief returns true if this source has a table attached, false otherwise
      ///
      bool hasTable() const  override;


      /// @brief retrieves a pointer to the active table for this source, if any.
      ///
      /// the returned table ptr may be null if this source doesn't have an active table.
      /// 
      GridTablePtr getTable() override;


      /// @brief assigns a table to this source.
      /// 
      /// triggers the TableInitialize event IF a non-null table ptr is passed.
      /// 
      /// If a null table ptr is passed, this grid will no longer fire events 
      /// until a subsequent call to setTable() passes a valid pointer.
      /// 
      void setTable(GridTablePtr table) override;


      /// @brief attaches an event sink to this source to receive event notifications
      ///
      /// detach() must be called when notifications no longer can/should be sent to 
      /// the subscriber, otherwise there is no way for the source to determine validity of 
      /// attached subscribers.
      /// 
      void attach(IGridTableEventSink* observer) override;


      /// @brief detach an event sink from this source to no longer receive event notifications
      ///
      void detach(IGridTableEventSink* observer) override;


      /// @brief this is called to signal that an event needs to be sent to all listeners
      ///
      bool signal(GridTableEvent event) override;
 

      /// @brief destructor
      ~GridTableSource() override = default;

   private:
      GridTablePtr m_table{};
      std::unordered_set<IGridTableEventSink*> m_observers{};
   
      /// @brief default ctor is private, use static create()
      GridTableSource() = default;

      // no copy/move/assign, this class is created on the heap.
      GridTableSource(const GridTableSource&) = delete;
      GridTableSource(GridTableSource&&) = delete;
      GridTableSource& operator=(const GridTableSource&) = delete;
      GridTableSource& operator=(GridTableSource&&) = delete;   
   };


}  // namespace  ctb::app