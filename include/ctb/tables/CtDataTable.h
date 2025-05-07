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
#include "ctb/tables/detail/TableRecord.h"

#include <vector>

namespace ctb
{
   using ctb::detail::TableRecord;

   /// @brief Type alias for a record in a CellarTracker data table
   /// 
   template <RecordTraitsType RecordTraits>
   using CtTableRecord = TableRecord<RecordTraits, CtProperty>;


   /// @brief Type alias for a data table of CellarTracker records
   /// 
   template <RecordTraitsType RecordTraits>
   using CtDataTable = std::vector<CtTableRecord<RecordTraits> >;


} // namespace ctb