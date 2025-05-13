#pragma once


#include "ctb/interfaces/IDataset.h"

#include "ctb/tables/detail/MultiMatchPropertyFilterMgr.h"
#include "ctb/tables/detail/PropertyFilter.h"
#include "ctb/tables/detail/SubStringFilter.h"
#include "ctb/tables/detail/TableSorter.h"

#include "ctb/model/CtDisplayColumn.h"

#include <vector>


namespace ctb
{
   /// @brief This is the data model class for interacting with CellarTracker datasets.
   /// 
   /// This class contains a dataset representing one of the CT user tables (Wine List, Pending Wines, etc)
   /// It provides access to all properties of the underlying dataset, but also has DisplayColumns, which are 
   /// the properties displayed in the main list-view. 
   /// 
   /// There are methods for searching or sorting value in the list view (i.e. DisplayColumns). There are also 
   /// filtering options for other properties.
   /// 
   template<typename DataTableT>
   class CtDataModel final : public IDataset
   {
   public:
      using base                = IDataset;
      using DataTable           = DataTableT;
      using DisplayColumn       = base::DisplayColumn;
      using DisplayColumns      = base::DisplayColumns;
      using MultiMatchFilterMgr = detail::MultiMatchPropertyFilterMgr<Prop, PropertyMap>;
      using MultiMatchFilter    = MultiMatchFilterMgr::Filter;
      using Prop                = base::Prop;
      using Property            = base::Property;
      using PropertyFilter      = detail::PropertyFilter<Prop, PropertyMap>;
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
         return DatasetPtr{ static_cast<IDataset*>(new CtDataModel{ std::move(data) }) };
      }

      /// @brief retrieves list of available SortConfigs, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      auto availableSorts() const -> TableSortSpan override
      {
         return Sorters;
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

      /// @brief Gets the collection of active display columns 
      /// 
      /// Note that some may be hidden and not visible.
      auto displayColumns() const -> const DisplayColumns& override
      { 
         return m_display_columns; 
      }

      /// @brief retrieves a list of available filters for this table.
      auto multiMatchFilters() const -> CtMultiMatchFilterSpan override
      {
         return MultiMatchFilters;
      }

      /// @brief Get a list of all distinct values from the table for the specified property.
      /// 
      /// This can be used to get filter values for match-filters.
      [[nodiscard]]
      auto getDistinctValues(CtProp prop_id) const -> PropertyValueSet override
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

      /// @brief Adds a match value filter for the specified column.
      ///
      /// a record must match at least one match_value for each property that has a filter 
      /// to be considered a match.
      /// 
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      auto addMultiMatchFilter(CtProp prop_id, const Property& match_value) -> bool override
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
      auto removeMultiMatchFilter(CtProp prop_id, const Property& match_value) -> bool override
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
         auto cols = displayColumns() | vws::transform([](const CtDisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
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

      /// @brief Used to enable/disable "in-stock only" filter to the data set, if supported.
      /// @return true if the filter was applied, false if it was not 
      auto setInStockFilter(bool enable) -> bool override
      {
         if (!hasProperty(Prop::QtyOnHand)) 
            return false;

         if (enable == m_instock_filter.enabled)  
            return true;

         m_instock_filter.enabled = enable;
         applyFilters();
         return true;
      }

      /// @brief Returns whether "in-stock only" filter to the data set, if supported.
      /// @return true if the filter is active, false if it was not 
      auto getInStockFilter() const -> bool override
      {
         if (!hasProperty(Prop::QtyOnHand) or !m_instock_filter.enabled)
            return false;

         return true;
      }

      /// @brief retrieves the minimum score filter value if active.
      auto getMinScoreFilter() const -> NullableDouble override
      {
         if (m_score_filter.enabled)
         {
            return m_score_filter.compare_val.asDouble();
         }
         return std::nullopt;
      }

      /// @brief set the minimum score filter
      auto setMinScoreFilter(NullableDouble min_score) -> bool override
      {
         if (min_score)
         {
            m_score_filter.enabled = true;
            m_score_filter.compare_val = *min_score;
         }
         else {
            m_score_filter.enabled = false;
         }
         applyFilters();
         return true;
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
      /// is by calling hasProperty()
      /// 
      /// The returned reference will remain valid until a modifying (non-const) method
      /// is called on this dataset, after which it may be invalid. You should copy-construct 
      /// a new object if you need to hold onto it for a while rather than holding the reference.
      /// 
      /// @return const reference to the requested property. It may be a null value, but it 
      ///  will always be a valid CtProperty&.
      auto getProperty(int rec_idx, CtProp prop_id) const -> const Property& override
      {
         if (rec_idx > filteredRecCount() or !hasProperty(prop_id))
            return null_prop;

         return m_current_view->at(static_cast<size_t>(rec_idx))[prop_id];
      }

      /// @brief returns the total number of records in the underlying dataset
      auto totalRecCount() const -> int64_t override
      {
         return std::ssize(m_data);
      }

      /// @brief returns the number of records with filters applied.
      auto filteredRecCount() const -> int64_t override
      {
         return std::ssize(*m_current_view);
      }

      /// @return the name of the CT table this dataset represents. Not meant to be 
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


      // default dtor, others are deleted since this object is meant to be heap-only
      ~CtDataModel() noexcept override = default;
      CtDataModel() = delete;
      CtDataModel(const CtDataModel&) = delete;
      CtDataModel(CtDataModel&&) = delete;
      CtDataModel& operator=(const CtDataModel&) = delete;
      CtDataModel& operator=(CtDataModel&&) = delete;

   private:
      /// @brief list of display columns that will show in the list view
      ///
      static inline const std::array DefaultDisplayColumns { 
         CtDisplayColumn{ Prop::WineAndVintage,                                constants::DISPLAY_COL_WINE     },
         CtDisplayColumn{ Prop::Locale,                                        constants::DISPLAY_COL_LOCALE   },
         CtDisplayColumn{ Prop::QtyTotal,   CtDisplayColumn::Format::Number,   constants::DISPLAY_COL_QTY      },
         CtDisplayColumn{ Prop::CtScore,    CtDisplayColumn::Format::Decimal,  constants::DISPLAY_COL_CT_SCORE },
         CtDisplayColumn{ Prop::MyScore,    CtDisplayColumn::Format::Decimal,  constants::DISPLAY_COL_MY_SCORE },
      };

      /// @brief the available sort orders for this table.
      ///
      static inline const std::array Sorters{ 
         TableSort{ { Prop::WineName, Prop::Vintage                       }, constants::SORT_OPTION_WINE_VINTAGE   },
         TableSort{ { Prop::Vintage,  Prop::WineName                      }, constants::SORT_OPTION_VINTAGE_WINE   },
         TableSort{ { Prop::Locale,   Prop::WineName,    Prop::Vintage    }, constants::SORT_OPTION_LOCALE_WINE    },
         TableSort{ { Prop::Region,   Prop::WineName,    Prop::Vintage    }, constants::SORT_OPTION_REGION_WINE    },
         TableSort{ { Prop::MyScore,  Prop::CtScore,     Prop::WineName,  }, constants::SORT_OPTION_SCORE_MY, true },
         TableSort{ { Prop::CtScore,  Prop::MyScore,     Prop::WineName,  }, constants::SORT_OPTION_SCORE_CT, true },
         TableSort{ { Prop::MyPrice,  Prop::WineName,    Prop::Vintage ,  }, constants::SORT_OPTION_MY_VALUE       }
      };

      /// @brief string filters that can be used on this table.
      ///
      static inline const std::array MultiMatchFilters{
         MultiMatchFilter{ Prop::Varietal,    constants::FILTER_VARIETAL   },
         MultiMatchFilter{ Prop::Country,     constants::FILTER_COUNTRY    },
         MultiMatchFilter{ Prop::Region,      constants::FILTER_REGION     },
         MultiMatchFilter{ Prop::Appellation, constants::FILTER_APPELATION },
         MultiMatchFilter{ Prop::Vintage,     constants::FILTER_VINTAGE    }
      };

      DataTable                        m_data{};                 // the underlying data records for this table.
      DataTable                        m_filtered_data{};        // we need a copy for the filtered data, so we can bind our views to it
      DataTable*                       m_current_view{};         // may point to m_data or m_filtered_data depending if filter is active
      DisplayColumns                   m_display_columns{};
      PropertyFilter                   m_instock_filter{};
      PropertyFilter                   m_score_filter{};
      MultiMatchFilterMgr              m_mm_filters{};
      TableSort                        m_current_sort{};
      std::optional<SubStringFilter>   m_substring_filter{};
      
      // private construction, use static factory method create();
      explicit CtDataModel(DataTable&& data) : 
         m_data{ std::move(data) },
         m_current_view{ &m_data },
         m_display_columns{ std::from_range, DefaultDisplayColumns },
         m_instock_filter{ Prop::QtyOnHand, std::greater<CtProperty>{}, uint16_t{0} },
         m_score_filter{ {Prop::CtScore, Prop::MyScore}, std::greater_equal<CtProperty>{}, constants::FILTER_SCORE_DEFAULT },
         m_current_sort{ availableSorts()[0] }
      {
         m_score_filter.enabled = false;
         m_instock_filter.enabled = false;
         sortData();
      }

      auto isDataFiltered() const -> bool 
      { 
         return m_current_view == &m_filtered_data; 
      }

      void applyFilters()
      {
         if (m_mm_filters.activeFilters() or m_instock_filter.enabled or m_score_filter.enabled)
         {
            m_filtered_data = vws::all(m_data) | vws::transform([](auto&& rec) { return rec.getProperties(); }) // filters work with property maps, not records
                                               | vws::filter(m_mm_filters)
                                               | vws::filter(m_instock_filter)
                                               | vws::filter(m_score_filter)
                                               | vws::transform([](auto&& map) { return Record{ map }; })        // back to record
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
         static const auto sort_adapter = [this](const Record& rec1, const Record& rec2) -> bool
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

};

} // namespace ctb