/*********************************************************************
 * @file       CtDataTable.h
 *
 * @brief      declaration for the CtProperty and CtDataTable types, which 
 *             are app-specific instantiations of our table classes for
 *             CellarTracker data files.
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once
#include "ctb/tables/detail/TableProperty.h"
#include "ctb/tables/detail/TableRecord.h"

#include <vector>

namespace ctb
{

   /// @brief Type alias for the property type used in CellarTracker data tables.
   ///
   using CtProperty = detail::TableProperty<uint16_t, uint64_t, double, std::string>;

   /// @brief useful if you need to return a const ref to a null property value
   ///
   static inline constexpr CtProperty null_prop{};


   /// @brief Type alias for a record in a CellarTracker data table
   /// 
   template <RecordTraitsType RecordTraits>
   using CtTableRecord = detail::TableRecord<RecordTraits, CtProperty>;


   /// @brief Type alias for a data table of CellarTracker records
   /// 
   template <RecordTraitsType RecordTraits>
   using CtDataTable = std::vector<CtTableRecord<RecordTraits> >;


} // namespace ctb