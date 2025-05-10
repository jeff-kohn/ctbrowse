/*********************************************************************
* @file       CtDataTable.h
*
* @brief      Declares the type alias CtProperty, which is used for 
*             our CellarTracker data tables. It also defines the 
*             CtProp enum, which contains the property identifiers
*             for all of the properties supported by the application.
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/TableProperty.h"

#include <boost/unordered/unordered_flat_map.hpp>

namespace ctb
{

   /// @brief Type alias for the property type used in CellarTracker data tables.
   ///
   using CtProperty = detail::TableProperty<uint16_t, uint64_t, double, std::string>;


   /// @brief useful if you need to return a const ref to a null property value
   ///
   static inline constexpr CtProperty null_prop{};


   /// @brief Enumeration for all of the properties supported by CellarTracker data tables.
   ///        Many properties are common across all tables, some are table-specific. 
   /// 
   enum class CtProp : uint16_t
   {
      iWineId,
      WineName,
      Vintage,
      WineAndVintage,   // calculated, not from CSV
      Country,
      Locale,
      Region,
      SubRegion,
      Appellation,
      Producer,    

      Category,
      Color,
      Varietal,         // actually MasterVarietal in the CSV
      Size,
            
      Currency,
      MyPrice,
      CtPrice,
      AuctionPrice,

      BeginConsume,
      EndConsume,
      
      QtyPending,
      QtyOnHand,
      QtyTotal,         // calculated, not from CSV

      CtScore,
      MyScore,

      // Specific to Pending Wines table
      PendingOrderNumber,
      PendingPrice,
      PendingPurchaseDate,
      PendingQtyOrdered,
      PendingStoreName,
      PendingDeliveryDate,
      PendingPurchaseId
   };


   using CtPropertyMap = boost::unordered_flat_map<CtProp, CtProperty>; 

} // namespace ctb