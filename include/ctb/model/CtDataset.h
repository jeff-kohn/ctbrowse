/*******************************************************************
* @file CtDataset.h
*
* @brief Header file declaring the CtDataset template class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/interfaces/IDataset.h"

#include "ctb/tables/detail/MultiMatchPropertyFilterMgr.h"
#include "ctb/tables/detail/PropertyFilter.h"
#include "ctb/tables/detail/PropertyFilterMgr.h"
#include "ctb/tables/detail/SubStringFilter.h"

#include <map>
#include <optional>

namespace ctb
{
   /// @brief This is the data model class for interacting with CellarTracker datasets.
   /// 
   /// This class implements a dataset representing one of the CT user tables (Wine List, Pending Wines, etc)
   /// It provides access to all properties of the underlying dataset, but also has ListColumns, which are 
   /// the properties displayed in the main list-view. 
   /// 
   /// THIS CLASS IS NOT THREADSAFE. UI code and UI-owned objects are inherently tied to the main thread
   /// in wxWidgets. Any background threads should work on their own data and send messages to the main thread.
   /// Access to the dataset should always be from main thread since multiple UI windows are holding reference 
   /// to it.
   /// 
   template<DataTableType DataTableT>
   class CtDataset final : public IDataset
   {
   public:
      using base                = IDataset;
      using DataTable           = DataTableT;
      using FieldSchama         = base::FieldSchema;
      using ListColumn          = base::ListColumn;
      using ListColumnSpan      = base::ListColumnSpan;
      using MultiMatchFilterMgr = detail::MultiMatchPropertyFilterMgr<Prop, PropertyMap>;
      using MultiMatchFilter    = MultiMatchFilterMgr::Filter;
      using Prop                = base::Prop;
      using PropertyVal         = base::PropertyVal;
      using PropertyFilter      = base::PropertyFilter;
      using PropertyFilterMgr   = detail::PropertyFilterMgr<Prop, base::PropertyMap>;
      using MaybePropFilter     = base::MaybePropFilter;
      using PropertyMap         = base::PropertyMap;
      using PropertyValueSet    = base::PropertyValueSet;
      using Record              = DataTable::value_type;
      using SubStringFilter     = detail::SubStringFilter<Record>;
      using TableSort           = base::TableSort;
      using TableSortSpan       = base::TableSortSpan;
      using Traits              = Record::Traits;

      /// @brief Create a data model object for the specified table
      /// 
      /// @return shared_ptr to the requested object
      static auto create(DataTable data) -> DatasetPtr
      {
         return DatasetPtr{ static_cast<IDataset*>(new CtDataset{ std::move(data) }) };
      }

      /// @brief Returns the name of the CT table this dataset represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      auto getTableName() const -> std::string_view override
      {
         return Traits::getTableName();
      }

      /// @brief Returns the TableId enum for this dataset's underlying table.
      auto getTableId() const -> TableId override
      {
         return Traits::getTableId();
      }

      /// @brief Retrieves a short text summary of the data in the table
      auto getDataSummary() const -> std::string override
      {
         if (m_current_view->empty())
            return {};

         switch (getTableId())
         {
            case TableId::Availability:
            {
               auto wines   = rowCount(true);
               auto bottles = foldValues(CtProp::RtdQtyDefault, int32_t{}, std::plus{});
               return ctb::format(constants::FMT_SUMMARY_AVAILABILITY, wines, bottles);
            }
            case TableId::Pending:
            {
               auto wines   = rowCount(true);
               auto stores  = getDistinctValues(CtProp::PendingStoreName).size();
               auto bottles = foldValues(CtProp::QtyPending, int32_t{}, std::plus{});
               return ctb::format(constants::FMT_SUMMARY_PENDING, wines, stores, bottles);
            }
            case TableId::List:
            {
               auto wines    = rowCount(true);
               auto on_hand  = foldValues(CtProp::QtyOnHand,  int32_t{}, std::plus{});
               auto on_order = foldValues(CtProp::QtyPending, int32_t{}, std::plus{});
               return ctb::format(constants::FMT_SUMMARY_MY_CELLAR, wines, on_hand, on_order);
            }
            default:
               return {};
         };
      }

      /// @brief Retrieves the schema information for a specified property.
      /// 
      /// @param prop_id - The identifier of the property whose schema is to be retrieved.
      /// @return An optional FieldSchema containing the schema information for the specified property, 
      ///  or std::nullopt if the property does not exist.
      auto getFieldSchema(Prop prop_id) const -> std::optional<FieldSchema> override
      {
         auto it = Traits::Schema.find(prop_id);
         if (it == Traits::Schema.end())
         {
            return std::nullopt;
         }
         return it->second;
      }

      /// @brief Gets the collection of active display columns 
      /// 
      /// Note that some may be hidden and not visible.
      auto listColumns() const -> CtListColumnSpan override
      { 
         return Traits::DefaultListColumns; 
      }

      /// @brief retrieves list of available sorters, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      auto availableSorts() const -> TableSortSpan override
      {
         return Traits::AvailableSorts;
      }

      /// @brief returns the currently active sort option
      auto activeSort() const -> const TableSort& override
      {
         return m_current_sort;
      }

      /// @brief specifies a new sort option
      void applySort(const TableSort& sort) override
      {
         if (sort != m_current_sort)
         {
            m_current_sort = sort;
            sortData();
         }
      }

      /// @brief retrieves a list of available filters for this table.
      auto multiMatchFilters() const -> CtMultiMatchFilterSpan override
      {
         return Traits::MultiMatchFilters;
      }

      /// @brief Adds a match value filter for the specified column.
      ///
      /// a record must match at least one match_value for each property that has a filter 
      /// to be considered a match.
      /// 
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      auto addMultiMatchFilter(CtProp prop_id, const PropertyVal& match_value) -> bool override
      {
         // if we somehow get passed a filter we already have, don't waste our time.
         if ( m_mm_filters.addFilter(prop_id, match_value) )
         {
            applyFilters();
            return true;
         }
         return false;
      }

      /// @brief removes a match value filter for the specified column.
      ///
      /// @return true if the filter was removed, false if it wasn't found
      auto removeMultiMatchFilter(CtProp prop_id, const PropertyVal& match_value) -> bool override
      {
         // if we somehow get passed filter that we aren't using, don't waste our time.
         if ( m_mm_filters.removeFilter(prop_id, match_value) )
         {
            applyFilters();
            return true;
         }
         return false;
      }

      /// @brief Apply a search filter that does substring matching on ANY column in the table view
      /// 
      /// If applied this filter will replace any previous substring filter, as there can be only
      /// one at a time.
      /// 
      /// @return true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. 
      auto filterBySubstring(std::string_view substr) -> bool override
      {
         // this overload searches all columns in the current list view, so get the prop_id's 
         auto cols = listColumns() | vws::transform([](const CtListColumn& disp_col) -> auto { return disp_col.prop_id; })
                                      | rng::to<std::vector>();

         return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
      }

      /// @brief Apply a search filter that does substring matching on the specified column
      ///
      /// If applied this filter will replace any previous substring filter, as there can be only
      /// one at a time.
      /// 
      /// @return true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. 
      auto filterBySubstring(std::string_view substr, CtProp prop_id) -> bool override
      {
         auto cols = std::vector{ prop_id };
         return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
      }

      /// @brief clear the substring filter
      void clearSubStringFilter() override
      {
         m_substring_filter = std::nullopt;
         applyFilters();
      }

      /// @brief Check if a filter with the specified name is applied to the dataset.
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return - true if there is a filter by the specified name, false otherwise.
      auto hasFilter(std::string_view filter_name) const -> bool override
      {
         return m_prop_filters.hasFilter(filter_name);
      }

      /// @brief Check if a filter with the specified name is applied to the dataset.
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return - true if there is a filter by the specified name, false otherwise.
      auto hasExactFilter(const PropertyFilter& filter) const -> bool override
      {
         return hasFilter(filter.name()) and filter == m_prop_filters.getFilter(filter.name()).value();
      }

      /// @brief Get the filter with the specified name that is applied to the dataset.
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return - the requested filter, or std::nullopt if not found
      auto getFilter(std::string_view filter_name) const  -> MaybePropFilter override
      {
         return m_prop_filters.getFilter(filter_name);
      }

      /// @brief Add a filter to the dataset. Existing filter with same name will NOT be replaced, use removeFilter() first.
      /// 
      /// @return true if the filter was added, false if not because a filter with that name already exists.
      auto addFilter(PropertyFilter filter) -> bool override
      {
         if (m_prop_filters.addFilter(filter) )
         {
            applyFilters();
            return true;
         }
         return false;
      }

      /// @brief Replace a named filter with an updated version
      /// 
      /// If the filter == existing, this will be a no-op. Otherwise existing 
      /// filter will be replaced with new and the dataset refreshed. 
      /// 
      /// If a filter by the same name doesn't already exist, this function is the
      /// same as calling addFilter()
      /// 
      /// This is more efficient than calling removeFilter/addFilter because the dataset
      /// will only be refreshed once.
      /// 
      /// @return true if filter was replaced or added, false if it no-op'd due to equality
      auto replaceFilter(const PropertyFilter& filter) -> bool override
      {
         if (hasExactFilter(filter))
            return false;

         m_prop_filters[filter.name()] = filter;
         applyFilters();
         return true;
      }

      /// @brief Remove the filter with the specified name
      /// 
      /// filter_name is case-sensitive
      /// 
      /// @return true if filter was removed; false if it wasn't found.
      auto removeFilter(std::string_view filter_name) -> bool override
      {
         if (m_prop_filters.removeFilter(filter_name))
         {
            applyFilters();
            return true;
         }
         return false;
      }

      /// @brief Remove all filters from the dataset
      /// 
      /// This removes property and multi-value filters.
      /// 
      /// @return true if at least one filter was removed, false if there were no filters
      auto removeAllFilters() -> bool override
      {
         bool got_one = false;
         if (m_mm_filters.activeFilters())
         {
            m_mm_filters.removeAllFilters();
            got_one = true;
         }
         if (m_prop_filters.activeFilters())
         {
            m_prop_filters.removeAllFilters();
            got_one = true;
         }
         return got_one;
      }

      /// @brief Check whether the current dataset supports the given property
      /// 
      /// Since getProperty() will return a null value for missing properties, calling this function
      /// is the only way to distinguish between a null property value and a property that is missing
      /// altogether.
      /// 
      /// @return True if the property is available, false if not.
      auto hasProperty(CtProp prop_id) const -> bool override
      {
         return Traits::hasProperty(prop_id);
      }

      /// @brief Retrieve a property for a specified record/row in the dataset
      /// 
      /// This function returns a reference to null for not-found properties. Since
      /// found properties could also have null value, the only way to differentiate
      /// is by checking hasProperty()
      /// 
      /// The returned reference will remain valid until a modifying (non-const) method
      /// is called on this dataset, after which it may be invalid. You should copy-construct 
      /// a new object if you need to hold onto it for a while rather than holding the reference.
      /// 
      /// @return const reference to the requested property. It may be a null value, but it 
      ///  will always be a valid CtPropertyVal&.
      auto getProperty(int rec_idx, CtProp prop_id) const noexcept(false) -> const PropertyVal & override
      {
         assert(rowCount(true) > rec_idx and "This is a logic bug, invalid index should never happen here.");

         const auto& record = m_current_view->at(static_cast<size_t>(rec_idx));
         return record[prop_id];
      }

      /// @brief Get a list of all distinct values from the table for the specified property.
      /// 
      /// This can be used to get filter values for match-filters.
      [[nodiscard]] auto getDistinctValues(CtProp prop_id) const -> PropertyValueSet override
      {
         PropertyValueSet values{};
         if (hasProperty(prop_id))
         {
            for (const Record& rec : m_data)
            {
               values.emplace(rec[prop_id]);
            }
         }
         return values;
      }

      /// @brief returns the number of rows in the underlying dataset
      /// @param filtered_only - if true, the count will only include rows matching any active filters.
      ///                        if false, the count will always be the raw/total number of rows
      auto rowCount(bool filtered_only) const -> int64_t override
      {
         return filtered_only ? std::ssize(*m_current_view) : std::ssize(m_data);
      }

      // default dtor, others are deleted since this object is meant to be heap-only
      ~CtDataset() noexcept override = default;
      CtDataset() = delete;
      CtDataset(const CtDataset&) = delete;
      CtDataset(CtDataset&&) = delete;
      CtDataset& operator=(const CtDataset&) = delete;
      CtDataset& operator=(CtDataset&&) = delete;

   private:
      using ListColumns          = std::vector<ListColumn>;
      using MaybeSubStringFilter = std::optional<SubStringFilter>;

      DataTable            m_data{};                 // the underlying data records for this table.
      DataTable            m_filtered_data{};        // we need a copy for the filtered data, so we can bind our views to it
      DataTable*           m_current_view{};         // may point to m_data or m_filtered_data depending if filter is active or not
      PropertyFilterMgr    m_prop_filters{};         // active property filters
      ListColumns          m_list_columns{};         // columns that will be displayed in the dataset list-view
      MultiMatchFilterMgr  m_mm_filters{};           // active multi-match filters
      MaybeSubStringFilter m_substring_filter{};
      TableSort            m_current_sort{};
      
      // private construction, use static factory method create();
      explicit CtDataset(DataTable&& data) : 
         m_data{ std::move(data) },
         m_current_view{ &m_data },
         m_list_columns{ std::from_range, Traits::DefaultListColumns },
         m_current_sort{ availableSorts()[0] }
      {
         sortData();
      }

      auto isDataFiltered() const -> bool 
      { 
         return m_current_view == &m_filtered_data; 
      }

      void applyFilters()
      {
         if (m_mm_filters.activeFilters() or m_prop_filters.activeFilters())
         {
            m_filtered_data = vws::all(m_data) | vws::transform([](auto&& rec) { return rec.getProperties(); }) // filters work with property maps, not records
                                               | vws::filter(m_mm_filters)
                                               | vws::filter(m_prop_filters)
                                               | vws::transform([](auto&& map) { return Record{ map }; })       // back to record
                                               | rng::to<std::vector>();
            m_current_view = &m_filtered_data;
         }
         else
         {
            m_current_view = &m_data;
         }

         if (m_substring_filter)
         {
            applySubStringFilter(*m_substring_filter);
         }
      }

      bool applySubStringFilter(const SubStringFilter& search_filter)
      {
         // clear any existing substring filter first, since we can only have one at a time. The 
         // new filter will be applied if there are any matches. If no matches, substring filter 
         // will be cleared (we don't restore old one because previous search text is no longer 
         // in the toolbar so it wouldn't make sense).
         m_substring_filter = {};
         applyFilters();
         auto filtered = vws::all(*m_current_view) | vws::filter(search_filter)
                                                   | vws::transform([] (auto&& rec) { return static_cast<const Record&>(rec); } )
                                                   | rng::to<std::vector>();
         if (filtered.empty())
            return false;

         m_substring_filter = search_filter;
         m_filtered_data.swap(filtered);
         m_current_view = &m_filtered_data;
         return true;
      }
      
      void sortData()
      {
         // the fact that our TableSorter class deals with PropertyMaps is a problem, because we actually need to 
         // sort a vector<TableRecordType>. But that would make a table-neutral CtTableSort impossible. So we have to use an 
         // adapter to allow us to use the sorter object.
         auto sort_adapter = [this](const Record& rec1, const Record& rec2) -> bool
            {
               return m_current_sort(rec1.getProperties(), rec2.getProperties());
            };

         // sort the data table, then re-apply any filters to the view. Otherwise we'd have to sort twice
         rng::sort(m_data, sort_adapter);
         applyFilters();
         if (m_substring_filter)
         {
            applySubStringFilter(*m_substring_filter);
         }
      }

      /// @brief Apply a left-fold to the values for the specified prop_id
      /// 
      /// Note that ValT should be a type that can be used to call CtPropertyVal::as<ValT>()
      /// 
      /// @param prop_id - dataset property to get values for
      /// @param initial_val - starting value for the fold, also determins return type
      /// @param fn - the function to apply for the fold operation
      /// @param filtered_only - if true, only rows matching active filters are included, if false ALL rows are included
      /// @return - the result of the fold 
      template<ArithmeticType ValT, typename FoldFunctionT>
      auto foldValues(Prop prop_id, ValT initial_val, FoldFunctionT fn, bool filtered_only = true) const -> ValT
      {
         constexpr auto getVal = [](auto&& prop) -> ValT
                                 { 
                                    return prop.as<ValT>().value_or(0); 
                                 };
         if (filtered_only)
         {
            return rng::fold_left(getSeriesFiltered(prop_id) | vws::transform(getVal), initial_val, fn);
         }
         else {
            return rng::fold_left(getSeriesRaw(prop_id)      | vws::transform(getVal), initial_val, fn);
         }
      }

      [[nodiscard]] auto getSeriesFiltered(CtProp prop_id) const
      {
         return vws::transform(*m_current_view, [prop_id](const Record& row) -> const CtPropertyVal&
                                                { 
                                                   return row[prop_id]; 
                                                });
      }

      [[nodiscard]] auto getSeriesRaw(CtProp prop_id) const 
      {
         return vws::transform(m_data, [prop_id](const Record& row) -> const CtPropertyVal&
                                       { 
                                          return row[prop_id]; 
                                       });
      }
   };

} // namespace ctb