/*******************************************************************
 * @file DatasetBase.h
 *
 * @brief Header file for the DatasetBase base class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/CtProperty.h"
#include "ctb/model/CtStringFilter.h"
#include "ctb/model/DisplayColumn.h"
#include "ctb/tables/detail/TableSorter.h"

#include <set>
#include <memory>


namespace ctb
{

   /// @brief Data model class that provides a base implementation for accessing CellarTracker data
   /// 
   /// Each CT table the app uses will derive form this class.
   /// 
   /// Users of this interface should gain access through IDatasetEventSource::getTable() or 
   /// using the ptr supplied when handling IDatasetEventSource::notify()
   /// 
   class IDataset
   {
   public:
      using Prop           = CtProp;
      using Property       = CtProperty;
      using PropertyMap    = CtPropertyMap;
      using DisplayColumn  = detail::DisplayColumn;
      using DisplayColumns = detail::DisplayColumns;
      using TableSort      = detail::TableSorter<CtProp, CtPropertyMap>;
      using TableSorts     = std::vector<TableSort>;

      /// @brief retrieves list of available SortConfigs, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      /// 
      virtual auto availableSorts() const -> const TableSorts& = 0;

      /// @brief returns the currently active sort option
      ///
      virtual auto activeSort() const -> const TableSort& = 0;

      /// @brief specifies a new sort option
      ///
      virtual void applySort(const TableSort& sort) = 0;

      /// @brief Gets the collection of active display columns 
      /// 
      /// Note that some may be hidden and not visible.
      /// 
      virtual auto displayColumns() const -> const DisplayColumns& = 0;

      /// @brief retrieves a list of available filters for this table.
      ///
      virtual auto availableStringFilters() const -> CtStringFilters = 0;

      /// @brief get a list of values that can be used to filter on a column in the table
      ///
      virtual auto getFilterMatchValues(CtProp prop_id) const -> StringSet = 0;

      /// @brief adds a match value filter for the specified column.
      ///
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      /// 
      /// a record must match at least one match_value for each property that 
      /// has a filter to be considered a match.
      /// 
      virtual auto addPropFilterString(CtProp prop_id, std::string_view match_value) -> bool = 0;

      /// @brief removes a match value filter for the specified column.
      ///
      /// @return true if the filter was removed, false if it wasn't found
      /// 
      virtual auto removePropFilterString(CtProp prop_id, std::string_view match_value) -> bool = 0;

      /// @brief filter does substring matching on ANY column in the table view
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. 
      ///
      virtual auto filterBySubstring(std::string_view substr) -> bool = 0;

      /// @brief sets filter that does substring matching on the specified column
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. 
      /// 
      virtual auto filterBySubstring(std::string_view substr, CtProp prop_id) -> bool = 0;

      /// @brief clear the substring filter
      ///
      virtual void clearSubStringFilter() = 0;

      /// @brief Used to enable/disable "in-stock only" filter to the data set, if supported.
      /// @return true if the filter was applied, false if it was not 
      /// 
      virtual auto setInStockFilter(bool enable) -> bool = 0;

      /// @brief Returns whether "in-stock only" filter to the data set, if supported.
      /// @return true if the filter is active, false if it was not 
      /// 
      virtual auto getInStockFilter() const -> bool = 0;

      /// @brief retrieves the minimum score filter value if active.
      /// 
      virtual auto getMinScoreFilter() const->NullableDouble = 0;

      /// @brief set the minimum score filter
      ///
      virtual auto setMinScoreFilter(NullableDouble min_score) -> bool = 0;

      /// @brief Check whether the current dataset supports the given property
      /// 
      /// Since getProperty() will return a null value for missing properties, calling this function
      /// is the only way to distinguish between a null property value and a property that is missing
      /// altogether.
      /// 
      /// @return True if the property is available, false if not.
      /// 
      virtual auto hasProperty(CtProp prop_id) const -> bool = 0;

      /// @brief Retrieve a property for a specified record/row in the  dataset
      /// 
      /// This function returns a reference to null for not-found properties. Since
      /// found properties could also have null value, the only way to differentiate
      /// is by calling
      /// 
      /// The returned reference will remain valid until a modifying (non-const) method
      /// is called on this dataset, after which it may be invalid. You should copy-construct 
      /// a new object if you need to hold onto it for a while rather than holding the reference.
      /// 
      /// @return const reference to the requested property. It may be a null value, but it 
      ///         will always be a valid CtProperty&.
      ///
      virtual auto getProperty(int rec_idx, CtProp prop_id) const -> const Property& = 0;

      /// @brief returns the total number of records in the underlying dataset
      ///
      virtual auto totalRecCount() const -> int64_t = 0;

      /// @brief returns the number of records with filters applied.
      ///
      virtual auto filteredRecCount() const -> int64_t = 0;

      /// @return the name of the CT table this dataset represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      /// 
      virtual auto getTableName() const -> std::string_view = 0;

      /// @brief destructor
      ///
      virtual ~IDataset() noexcept = default;
   };


   /// @brief the smart-ptr-to-base that's used to work with the IDataset interface 
   ///
   using DatasetPtr = std::shared_ptr<IDataset>;


}  // namespace ctb
