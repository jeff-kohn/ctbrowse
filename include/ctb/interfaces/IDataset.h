/*******************************************************************
 * @file IDataset.h
 *
 * @brief Header file for the IDataset interface
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/table_data.h"
#include "ctb/tables/CtSchema.h"

#include <memory>
#include <optional>
#include <string_view>


namespace ctb
{



   /// @brief Data model class that provides a base implementation for accessing CellarTracker data files
   /// 
   class IDataset
   {
   public:
      using FieldSchema       = CtFieldSchema;
      using Prop              = CtProp;
      using PropertyVal       = CtPropertyVal;
      using PropertyFilter    = CtPropertyFilter;
      using MaybeFilter       = std::optional<PropertyFilter>;
      using PropertyMap       = CtPropertyMap;
      using PropertyValueSet  = CtPropertyValueSet;
      using ListColumn        = CtListColumn;
      using ListColumnSpan    = CtListColumnSpan;
      using TableSort         = CtTableSort;
      using TableSortSpan     = CtTableSortSpan;
      using PropertyRef       = std::reference_wrapper<const PropertyVal>;
      using PropertyRefs      = std::vector<PropertyRef>;

      /// @return the name of the CT table this dataset represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      [[nodiscard]] virtual auto getTableName() const -> std::string_view = 0;

      /// @brief Returns the TableId enum for this dataset's underlying table.
      virtual auto getTableId() const -> TableId = 0;

      /// @brief Retrieves a short text summary of the data in the table
      virtual auto getDataSummary() const -> std::string = 0;

      /// @brief Retrieves the schema information for a specified property.
      /// 
      /// @param prop_id - The identifier of the property whose schema is to be retrieved.
      /// @return An optional FieldSchema containing the schema information for the specified property, 
      ///  or std::nullopt if the property does not exist.
      [[nodiscard]] virtual auto getFieldSchema(Prop prop_id) const -> std::optional<FieldSchema> = 0;

      /// @brief Gets the collection of columns for the list display
      /// 
      /// Note that some may be hidden and not visible.
      virtual auto listColumns() const -> ListColumnSpan = 0;

      /// @brief Check whether the current dataset supports the given property
      /// 
      /// Since getProperty() will return a null value for missing properties, calling this function
      /// is the only way to distinguish between a null property value and a property that is missing
      /// altogether.
      /// @brief retrieves list of available sorters, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      virtual auto availableSorts() const -> TableSortSpan = 0;

      /// @brief returns the currently active sort option
      virtual auto activeSort() const -> const TableSort& = 0;

      /// @brief specifies a new sort option
      virtual void applySort(const TableSort& sort) = 0;

      /// @brief retrieves a list of available filters for this dataset.
      virtual auto availableMultiValueFilters() const -> CtMultiValueFilterSpan = 0;

      /// @brief Adds a match value filter for the specified column.
      ///
      /// a record must match at least one match_value for each property that has a filter 
      /// to be considered a match.
      /// 
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      virtual auto addMultiValueFilter(CtProp prop_id, const PropertyVal& match_value) -> bool = 0;

      /// @brief removes a match value filter for the specified column.
      ///
      /// @return true if the filter was removed, false if it wasn't found
      virtual auto removeMultiValueFilter(CtProp prop_id, const PropertyVal& match_value) -> bool = 0;

      /// @brief Apply a search filter that does substring matching on ANY column in the dataset view
      /// 
      /// If applied this filter will replace any previous substring filter, as there can be only
      /// one at a time.
      /// 
      /// @return true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. 
      virtual auto filterBySubstring(std::string_view substr) -> bool = 0;

      /// @brief Apply a search filter that does substring matching on the specified column
      ///
      /// If applied this filter will replace any previous substring filter, as there can be only
      /// one at a time.
      /// 
      /// @return true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. 
      virtual auto filterBySubstring(std::string_view substr, CtProp prop_id) -> bool = 0;

      /// @brief clear the substring filter
      virtual void clearSubStringFilter() = 0;

      /// @brief Check if a filter with the specified name is applied to the dataset.
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return - true if there is a filter by the specified name, false otherwise.
      virtual auto hasFilter(std::string_view filter_name) const -> bool = 0;

      /// @brief Get the filter with the specified name that is applied to the dataset.
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return - the requested filter, or std::nullopt if not found
      [[nodiscard]] virtual auto getPropFilter(std::string_view filter_name) const -> std::optional<PropertyFilter> = 0;

      /// @brief Add the supplied filter to the dataset, replacing any existing filter with the same name
      /// @return true if resulting record count is > 0, false if resulting record count is == 0
      virtual auto applyPropFilter(const PropertyFilter& filter) -> bool = 0;

      /// @brief Remove the filter with the specified name
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return true if filters was removed, false if it doens't exist.
      virtual auto removePropFilter(const std::string& filter_name) -> bool = 0;

      /// 
      /// @return True if the property is available, false if not.
      virtual auto hasProperty(CtProp prop_id) const -> bool = 0;

      /// @brief Retrieve a property for a specified record/row in the dataset
      /// 
      /// This function returns a reference to null for not-found properties. Since
      /// found properties could also have null value, the only way to differentiate
      /// is by calling hasProperty()
      /// 
      /// The returned reference will remain valid until a modifying (non-const) method
      /// is called on this dataset, after which it may be invalid. You should copy-construct 
      /// a new object if you need to hold onto it for a while rather than holding the reference.
      /// 
      /// @return const reference to the requested property. It may be a null value, but it 
      ///         will always be a valid CtPropertyVal&.
      [[nodiscard]] virtual auto getProperty(int rec_idx, CtProp prop_id) const -> const PropertyVal& = 0;

      /// @brief Get a list of all distinct values from the dataset for the specified property.
      /// 
      /// This can be used to get filter values for match-filters. If filtered_only is true, only records matching
      /// the active filters will be included. If filtered_only is false, all records will be included.
      [[nodiscard]] virtual auto getDistinctValues(CtProp prop_id, bool filtered_only) const -> PropertyValueSet = 0;

      [[nodiscard]]
      /// @brief returns the number of records in the underlying dataset
      /// @param filtered_only - if true, only records matching currently active filters will be counted. If false, 
      virtual auto rowCount(bool filtered_only = true) const -> int64_t = 0;

      /// @brief destructor
      virtual ~IDataset() noexcept = default;
   };


   /// @brief the smart-ptr-to-base that's used to work with the IDataset-derived datasets
   using DatasetPtr = std::shared_ptr<IDataset>;


}  // namespace ctb
