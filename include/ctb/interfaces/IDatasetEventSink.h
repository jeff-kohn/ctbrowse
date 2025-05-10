/*******************************************************************
* @file IDatasetEventSink.h
*
* @brief Header file defining IDatasetEventSink interface
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/interfaces/DatasetEvent.h"


namespace ctb
{
   /// @brief listener interface for classes that want to receive notification events about a dataset.
   ///
   struct IDatasetEventSink
   {
      /// @brief called to notify the sink that a dataset event has occurred.
      ///
      /// The supplied pointer will remain valid until a subsequent event
      /// notification of type TableInitialize is received.
      /// 
      /// Event is passed by value because it can't be const-ref and we want to be able to pass
      /// temporaries. Also makes clear that changes to the event itself don't
      /// propagate back to caller.
      /// 
      virtual void notify(DatasetEvent event) = 0;


      /// @brief virtual destructor
      ///
      virtual ~IDatasetEventSink() noexcept = default;
   };

} // namespace ctb