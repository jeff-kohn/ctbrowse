/*******************************************************************
 * @file DatasetBase.h
 *
 * @brief Header file for the IDataset base class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/CtStringFilter.h"

#include <ctb/CtProperty.h>
#include <wx/dataview.h>

#include <set>
#include <memory>


namespace ctb::app
{
   /// @brief Struct that contains name, index and direction of a sort config.
   ///        Used by DatasetBase class.
   ///
   struct CtSortConfig
   {
      int               sorter_index{}; // index of the sorter in model::Sorters, not a property index
      std::string_view  sorter_name{};
      bool              ascending{ true };

      [[nodiscard]] std::strong_ordering operator<=>(const CtSortConfig&) const = default;
   };


   /// @brief our "data model", provides a base class interface for accessing CellarTracker data
   /// 
   /// users of this interface should gain access through 
   /// IDatasetEventSource::getTable() or using the ptr supplied when
   /// handling IDatasetEventSource::notify()
   /// 
   class IDataset : public wxDataViewVirtualListModel
   {
   public:
      using SortConfigs = std::vector<CtSortConfig>;

      /// @brief retrieves list of available SortConfigs, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      /// 
      virtual auto availableSortConfigs() const -> SortConfigs = 0;

      /// @brief returns the currently active sort option
      ///
      virtual auto activeSortConfig() const -> CtSortConfig = 0;

      /// @brief specifies a new sort option, triggers DatasetEvent::Sort
      ///
      virtual void applySortConfig(const CtSortConfig& config) = 0;

      /// @brief retrieves a list of available filters for this table.
      ///
      virtual auto availableStringFilters() const -> CtStringFilters = 0;

      /// @brief get a list of values that can be used to filter on a column in the table
      ///
      virtual auto getFilterMatchValues(int prop_idx) const -> StringSet = 0;

      /// @brief adds a match value filter for the specified column.
      ///
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      /// 
      /// a record must match at least one match_value for each property that 
      /// has a filter to be considered a match.
      /// 
      virtual auto addPropFilterString(int prop_idx, std::string_view match_value) -> bool = 0;

      /// @brief removes a match value filter for the specified column.
      ///
      /// @return true if the filter was removed, false if it wasn't found
      /// 
      virtual auto removePropFilterString(int prop_idx, std::string_view match_value) -> bool = 0;

      /// @brief filter does substring matching on ANY column in the table view
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers DatasetEvent::SubStringFilter
      ///
      virtual auto filterBySubstring(std::string_view substr) -> bool = 0;

      /// @brief sets filter that does substring matching on the specified column
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers DatasetEvent::SubStringFilter
      /// 
      virtual auto filterBySubstring(std::string_view substr, int col_idx) -> bool = 0;

      /// @brief clear the substring filter
      ///
      /// triggers DatasetEvent::SubStringFilter
      /// 
      virtual void clearSubStringFilter() = 0;

      /// @brief apples "in-stock only" filter to the data set, if supported.
      /// @return true if the filter was applied, false if it was not 
      /// 
      /// not all datasets support this filter, you can check by calling hasInStockFilter()
      /// 
      virtual auto enableInStockFilter(bool enable) -> bool = 0;

      /// @brief indicates whether the dataset supports filtering to in-stock only 
      /// @return true if supported, false otherwise.
      /// 
      virtual constexpr auto hasInStockFilter() const -> bool = 0;

      /// @brief retrieves the minimum score filter value if active.
      /// 
      virtual auto getMinScoreFilter() const -> NullableDouble = 0;

      /// @brief set the minimum score filter
      ///
      virtual auto setMinScoreFilter(NullableDouble min_score) -> bool = 0;

      /// @brief Retrieve a property from the underlying dataset
      /// 
      /// Since we don't have table-neutral indices to use, this lookup has to be done by
      /// property name as a string that corresponds to correct enum. 
      /// 
      /// @return the property value formatted as a string if found, std::nullopt if not found 
      ///
      virtual auto getDetailProp(int row_idx, std::string_view prop_name) const -> const CtProperty& = 0;

      /// @return the name of the CT table this dataset represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      /// 
      virtual auto getTableName() const -> std::string_view = 0;

      /// @brief returns the total number of records in the underlying dataset
      ///
      virtual int totalRowCount() const = 0;


      /// @brief returns the number of records with filters applied.
      ///
      virtual int filteredRowCount() const  = 0;

      /// @brief destructor
      ///
      ~IDataset() noexcept override
      {}
   };


   /// @brief the smart-ptr-to-base that's used to work with the IDataset interface 
   ///
   using IDatasetPtr = wxObjectDataPtr<IDataset>;


}  // namespace ctb::app