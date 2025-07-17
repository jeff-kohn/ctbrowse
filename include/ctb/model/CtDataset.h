/*******************************************************************
* @file CtDataset.h
*
* @brief Header file declaring the CtDataset template class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "ctb/interfaces/IDataset.h"

#include "ctb/tables/detail/PropertyFilter.h"
#include "ctb/tables/detail/FilterManager.h"
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
   /// THIS CLASS IS NOT THREADSAFE. It doens't need to be since UI code in GUI frameworks like wxWidgets is tied to main message thread. 
   /// Any background threads should work on their own data and send messages to the main thread/window. Access to the dataset should 
   /// always be from main thread since multiple UI windows are holding references to it.
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
      using MultiValueFilterMgr = base::MultiValueFilterMgr;
      using Prop                = base::Prop;
      using PropertyVal         = base::PropertyVal;
      using PropertyFilterMgr   = base::PropertyFilterMgr;
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

      /// @brief Returns the TableId enum for this dataset's underlying table.
      auto getTableId() const -> TableId override
      {
         return Traits::getTableId();
      }

      /// @brief Returns the filter_name of the CT table this dataset represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      auto getTableName() const -> std::string_view override
      {
         return Traits::getTableName();
      }

      /// @brief Returns the saved collection name.
      ///
      /// defaults to getTableDescription()
      /// 
      /// @return A reference to the collection name string.
      auto getCollectionName() const -> const std::string & override
      {
         return m_collection_name;
      }

      /// @brief Sets the name of the collection.
      /// @param name - The new name to assign to the collection.
      void setCollectionName(std::string_view name) override
      {
         m_collection_name = name;
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
               auto stores  = getDistinctValues(CtProp::PendingStoreName, true).size();
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
            case TableId::Consumed:
            {
               std::string result{ constants::SUMMARY_EMPTY };
               auto wine_count = rowCount(true);
               if (wine_count)
               {
                  // get earliest year (return values are sorted).
                  auto first_year = getDistinctValues(CtProp::ConsumeYear, true).begin()->asUInt16().value_or(0);
                  result = ctb::format(constants::FMT_SUMMARY_CONSUMED, wine_count, first_year);
               }
               return result;
            }
            default:
               return {};
         }
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
      auto availableMultiValueFilters() const -> CtMultiValueFilterSpan override
      {
         return Traits::MultiValueFilters;
      }

      /// @brief Retrieve the filter manager for PropertyFilter's 
      auto propFilters() -> PropertyFilterMgr& override
      {
         return m_prop_filters;
      }

      /// @brief Retrieve the filter manager for PropertyFilter's 
      auto propFilters() const -> const PropertyFilterMgr& override
      {
         return m_prop_filters;
      }

      /// @brief Retrieve the filter manager for MultiValueFilter's
      auto multivalFilters() -> MultiValueFilterMgr& override
      {
         return m_mval_filters;
      }

      /// @brief Retrieve the filter manager for MultiValueFilter's
      auto multivalFilters() const -> const MultiValueFilterMgr& override
      {
         return m_mval_filters;
      }

      /// @brief Apply a search filter that does substring matching on ANY column in the table view
      /// 
      /// If applied this filter will replace any previous substring filter, as there can be only
      /// one at a time.
      /// 
      /// @return true if filter was applied, false if there were no matches in which
      ///  case the filter was not applied. 
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
      ///  case the filter was not applied. 
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
      /// @return const reference to the requested property. It may contain a null value, but it 
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
      [[nodiscard]] auto getDistinctValues(CtProp prop_id, bool use_current_filters) const -> PropertyValueSet override
      {
         PropertyValueSet values{};
         if (hasProperty(prop_id))
         {
            for (const Record& rec : use_current_filters? *m_current_view : m_data)
            {
               values.emplace(rec[prop_id]);
            }
         }
         return values;
      }

      /// @brief Get a list of all distinct values from the dataset for the specified property.
      /// 
      /// This can be used to get filter values for match-filters. The supplied custom_filter will be used to limit 
      /// values to only those from records that match the filter.
      [[nodiscard]] auto getDistinctValues(CtProp prop_id, std::function<bool(const PropertyMap&)> custom_filter) const -> PropertyValueSet override
      {
         auto extractor = [prop_id](const PropertyMap& map)
            {
               PropertyVal result{};

               auto it = map.find(prop_id);
               if (it != map.end())
               {
                  result = it->second;
               }
               return result;
            };

         return vws::all(m_data) | vws::transform([](auto&& rec) { return rec.getProperties(); }) 
                                 | vws::filter(custom_filter)
                                 | vws::transform(extractor)
                                 | rng::to<PropertyValueSet>();
      }

      /// @brief returns the number of rows in the underlying dataset
      /// @param filtered_only - if true, the count will only include rows matching any active filters.
      ///                        if false, the count will always be the raw/total number of rows
      auto rowCount(bool filtered_only) const -> int64_t override
      {
         return filtered_only ? std::ssize(*m_current_view) : std::ssize(m_data);
      }

      void freezeData() noexcept override
      {
         m_frozen = true;
      }

      void unfreezeData() override
      {
         if (!m_frozen)
            return;

         m_frozen = false;
         sortData();       // also applies filters, so it's a full refresh
      }

      // default dtor, others are deleted since this object is meant to be heap-only
      ~CtDataset() noexcept override
      {
         m_mval_filters.unsubscribeChanges();
         m_prop_filters.unsubscribeChanges();
      }

      CtDataset() = delete;
      CtDataset(const CtDataset&) = delete;
      CtDataset(CtDataset&&) = delete;
      CtDataset& operator=(const CtDataset&) = delete;
      CtDataset& operator=(CtDataset&&) = delete;

   private:
      using ListColumns          = std::vector<ListColumn>;
      using MaybeSubStringFilter = std::optional<SubStringFilter>;

      bool                 m_frozen{ false };        // If true, data will not requery when filter/sort options are changed, until unfreezeData() is called.
      DataTable            m_data{};                 // the underlying data records for this table.
      DataTable            m_filtered_data{};        // we need a copy for the filtered data, so we can bind our views to it
      DataTable*           m_current_view{};         // may point to m_data or m_filtered_data depending if filter is active or not
      ListColumns          m_list_columns{};         // columns that will be displayed in the dataset list-view
      MultiValueFilterMgr  m_mval_filters{};         // active multi-match filters
      PropertyFilterMgr    m_prop_filters{};         // active property filters
      MaybeSubStringFilter m_substring_filter{};
      std::string          m_collection_name{};
      TableSort            m_current_sort{};
      
      // private construction, use static factory method create();
      explicit CtDataset(DataTable&& data) : 
         m_data{ std::move(data) },
         m_current_view{ &m_data },
         m_list_columns{ std::from_range, Traits::DefaultListColumns },
         m_collection_name{ getTableDescription(getTableId()) },
         m_current_sort{ availableSorts()[0] }
      {
         sortData();
         m_mval_filters.subscribeChanges([this]{ applyFilters(); });
         m_prop_filters.subscribeChanges([this]{ applyFilters(); });
      }

      auto isDataFiltered() const -> bool 
      { 
         return m_current_view == &m_filtered_data; 
      }

      void applyFilters()
      {
         if (m_frozen)
            return;

         if (m_mval_filters.empty() and m_prop_filters.empty())
         {
            m_current_view = &m_data;
         }
         else{
            // filters work with property maps, not records (since tables themselves are type-erased), 
            // so we have to transform back/forth to apply the filters, which is fortunately pretty inexpensive
            // since we're copying records into m_filtered_data anyways. Since our filter mgrs can be fairly
            // expensive to copy, use std::ref
            m_filtered_data = vws::all(m_data) | vws::transform([](auto&& rec) { return rec.getProperties(); }) 
                                               | vws::filter(std::ref(m_mval_filters))
                                               | vws::filter(std::ref(m_prop_filters))
                                               | vws::transform([](auto&& map) { return Record{ map }; })       // back to record
                                               | rng::to<std::vector>();
            m_current_view = &m_filtered_data;
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
         // in the toolbar, which would be confusing).
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
         // sort a vector<Recordc>. But that would make a table-neutral CtTableSort impossible. So we have to use an 
         // adapter to allow us to use the sorter object.
         auto sort_adapter = [this](const Record& rec1, const Record& rec2) -> bool
            {
               return m_current_sort(rec1.getProperties(), rec2.getProperties());
            };

         // sort the data table, then re-apply any filters to the view. Otherwise we'd have to sort twice
         rng::sort(m_data, sort_adapter);
         applyFilters();
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
                                    return prop.template as<ValT>().value_or(ValT{});
                                 };
         if (filtered_only)
         {
            return rng::fold_left(getSeriesFiltered(prop_id) | vws::transform(getVal), initial_val, fn);
         }
         else 
         {
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