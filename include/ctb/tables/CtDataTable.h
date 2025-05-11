/*********************************************************************
 * @file       CtDataTable.h
 *
 * @brief      Declares the type alias CtDataTable, which is the basis
 *             for our app-specific instantiations of data table classes 
 *             for CellarTracker data files.
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/tables/CtProperty.h"
#include "ctb/tables/TableRecord.h"

#include <vector>

namespace ctb
{

   /// @brief Type alias for a record in a CellarTracker data table
   /// 
   template <RecordTraitsType RecordTraits>
   using CtTableRecord = TableRecord<RecordTraits, CtPropertyMap>;


   /// @brief Type alias for a data table of CellarTracker records
   /// 
   template <RecordTraitsType RecordTraits>
   using CtDataTable = std::vector<CtTableRecord<RecordTraits> >;



   /// @brief type alias for Ct-specific CtDisplayColumn types
   ///
   //using CtDisplayColumn   = CtDisplayColumn<CtProp>;
   //using CtDisplayColumns  = DisplayColumns<CtProp>;


} // namespace ctb