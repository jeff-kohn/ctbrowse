#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/detail/FieldSchema.h"
#include "ctb/tables/detail/ListColumn.h"
#include "ctb/tables/detail/MultiMatchPropertyFilter.h"
#include "ctb/tables/detail/PropertyFilter.h"
#include "ctb/tables/detail/TableProperty.h"
#include "ctb/tables/detail/TableRecord.h"
#include "ctb/tables/detail/TableSorter.h"

#include <boost/unordered/unordered_flat_map.hpp>
#include <chrono>


namespace ctb
{
   /// @brief Enumeration for all of the properties supported by CellarTracker data tables.
   /// 
   /// Many properties are common across all tables, some are table-specific. 
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
      CtBeginConsume,
      EndConsume,
      CtEndConsume,


      QtyPending,
      QtyOnHand,
      QtyTotal,         // string value, calculated as Pending + OnHand, not from CSV (except for ReadyToDrink)
      QtyPurchased,     
      QtyConsumed,      

      CtScore,
      MyScore,

      // Specific to Pending Wines table
      PendingOrderNumber,
      PendingPrice,
      PendingOrderDate,
      PendingOrderQty,
      PendingStoreName,
      PendingDeliveryDate,
      PendingPurchaseId,

      // Specific to Ready To Drink Report
      RtdQtyDefault,
      RtdQtyLinear,
      RtdQtyBellCurve,
      RtdQtyEarlyCurve,
      RtdQtyLateCurve,
      RtdQtyFastMaturing,
      RtdQtyEarlyAndLate,
      RtdQtyBottlesPerYear,
      RtdConsumed,

      // consumed bottles table
      iConsumeId,
      iTastingNoteId,
      ConsumeDate,
      ConsumeReason,
      ConsumeNote,
      PurchaseNote,
      BottleNote,
      Location,
      Bin,
   };

   
   /// @brief  Type alias for data types supported by CtFieldSchema
   using CtPropType = detail::PropType;

   
   /// @brief Type alias for a CtProp-based field schema object 
   using CtFieldSchema = detail::FieldSchema<CtProp>;

   // promote non-template PropType to ctb namespace.
   using detail::PropType;
  
   
   /// @brief Type alias for the property type used in CellarTracker data tables
   using CtProperty = detail::TableProperty<std::string, uint16_t, uint64_t, double, std::chrono::year_month_day>;

   
   /// @brief Type alias for a sorted collection of property values
   using CtPropertyValueSet = std::set<CtProperty>;


   /// @brief 'Null' property value. Can be used when returning reference that doesn't have lifetime issues.
   static inline constexpr CtProperty ct_null_prop{};


   /// @brief Type alias for a table record indexed on a property enum instead of zero-based index
   using CtPropertyMap = boost::unordered_flat_map<CtProp, CtProperty>; 


   /// @brief Type alias for a CtProp-based record in a CellarTracker data table
   template <RecordTraitsType RecordTraits>
   using CtTableRecord = detail::TableRecord<RecordTraits, CtPropertyMap>;


   /// @brief Type alias for a CtProp-based ListColumn in a CellarTracker data table
   using CtListColumn = detail::ListColumn<CtProp>;


   /// @brief Type alias for a readonly span/view of CtListColumns.
   using CtListColumnSpan = std::span<const CtListColumn>;


   /// @brief Type alias for a CtProp-based data table of CellarTracker records
   template <RecordTraitsType RecordTraits>
   using CtDataTable = std::vector<CtTableRecord<RecordTraits> >;


   /// @brief Type alias for CtProp-based multi-match filter
   using CtMultiMatchFilter = detail::MultiMatchPropertyFilter<CtProp, CtPropertyMap>;


   /// @brief Type alias for CtProp-based multi-match filter
   using CtMultiMatchFilterSpan = std::span<const CtMultiMatchFilter>;
   

   /// @brief Type alias for a CtProp-based table property filter
   using CtPropertyFilter = detail::PropertyFilter<CtProp, CtPropertyMap>;


   /// @brief Type alias for CtProp-based table sorter
   using CtTableSort = detail::TableSorter<CtProp, CtPropertyMap>;
   

   /// @brief Type alias for span of CtTableSorts
   using CtTableSortSpan = std::span<const CtTableSort>;


} // namespace ctb