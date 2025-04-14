#pragma once
#include "App.h"
#include "interfaces/IDataset.h"
#include <ctb/CtRecordImpl.h>
#include <ctb/DisplayColumn.h>
#include <ctb/PropFilter.h>
#include <ctb/PropStringFilterMgr.h>
#include <ctb/SubStringFilter.h>
#include <ctb/TableSorter.h>

#include <vector>

namespace ctb::app
{
   /// @brief This is the data model class for displaying and interacting with CellarTracker datasets.
   /// 
   /// This class contains a dataset representing one of the CT user tables (Wine List, Pending Wines, etc)
   /// It provides access to all properties of the underlying dataset, but also has DisplayColumns, which are 
   /// the properties displayed in the main listview/grid. 
   /// 
   /// There are methods for searching or sorting value in the list view (i.e. DisplayColumns). There are also 
   /// filtering options for other properties.
   /// 
   template<typename DatasetT>
   class CellarTrackerDataModel : public IDataset
   {
   public:
      using Dataset             = DatasetT;
      using Record              = Dataset::value_type;
      using PropId              = Record::PropId;
      using Property            = Record::TableProperty;
      using Traits              = Record::Traits;

      using DisplayColumn       = DisplayColumn<Record>;
      using DisplayColumns      = std::vector<DisplayColumn>;
      using PropFilter          = PropFilter<Record, Property>;
      using PropStringFilterMgr = PropStringFilterMgr<Record>;
      using SubStringFilter     = SubStringFilter<Record>;
      using TableSort           = TableSorter<Record>;
      using SortConfigs         = IDataset::SortConfigs;


      /// @brief list of display columns that will show in the list view
      ///
      static inline const std::array DefaultDisplayColumns { 
         DisplayColumn{ PropId::WineAndVintage,                              constants::COL_WINE     },
         DisplayColumn{ PropId::Locale,                                      constants::COL_LOCALE   },
         DisplayColumn{ PropId::TotalQty,   DisplayColumn::Format::Number,   constants::COL_QTY      },
         DisplayColumn{ PropId::CTScore,    DisplayColumn::Format::Decimal,  constants::COL_CT_SCORE },
         DisplayColumn{ PropId::MYScore,    DisplayColumn::Format::Decimal,  constants::COL_MY_SCORE },
      };

      /// @brief the available sort orders for this table.
      ///
      static inline const std::array Sorters{ 
         TableSort{ { PropId::WineName, PropId::Vintage                         }, constants::SORT_OPTION_WINE_VINTAGE   },
         TableSort{ { PropId::Vintage,  PropId::WineName                        }, constants::SORT_OPTION_VINTAGE_WINE   },
         TableSort{ { PropId::Locale,   PropId::WineName,    PropId::Vintage    }, constants::SORT_OPTION_LOCALE_WINE    },
         TableSort{ { PropId::Region,   PropId::WineName,    PropId::Vintage    }, constants::SORT_OPTION_REGION_WINE    },
         TableSort{ { PropId::MYScore,  PropId::CTScore,     PropId::WineName,  }, constants::SORT_OPTION_SCORE_MY       },
         TableSort{ { PropId::CTScore,  PropId::MYScore,     PropId::WineName,  }, constants::SORT_OPTION_SCORE_CT       },
         TableSort{ { PropId::Price,    PropId::WineName,    PropId::Vintage ,  }, constants::SORT_OPTION_MY_VALUE       }
      };

      /// @brief string filters that can be used on this table.
      ///
      static inline const std::array StringFilters{
         CtStringFilter{ constants::FILTER_VARIETAL,   static_cast<int>(PropId::MasterVarietal) },
         CtStringFilter{ constants::FILTER_COUNTRY,    static_cast<int>(PropId::Country)        },
         CtStringFilter{ constants::FILTER_REGION,     static_cast<int>(PropId::Region)         },
         CtStringFilter{ constants::FILTER_APPELATION, static_cast<int>(PropId::Appellation)    }
      };

      static auto create(Dataset data) -> IDatasetPtr
      {
         return IDatasetPtr{ static_cast<IDataset*>(new CellarTrackerDataModel{ std::move(data) }) };
      }

      /// @brief  get a list of the columns that will be displayed in the grid
      /// 
      /// the columns are in the order they will be displayed.
      ///
      auto getDisplayColumns() const -> DisplayColumns 
      { 
         return m_display_columns; 
      }

      /// @brief retrieves list of available SortConfigs, in order of display
      /// 
      /// the index in this vector corresponds to the index in the sort_index
      /// property.
      /// 
      auto availableSortConfigs() const -> SortConfigs override
      {
         SortConfigs configs(Sorters.size());
         for (const auto&& [i, table_sort] : vws::enumerate(Sorters))
         {
            configs.emplace_back(CtSortConfig{ static_cast<int>(i), table_sort.sort_name  });
         }
         return configs;
      }

      /// @brief returns the currently active sort option
      ///
      auto activeSortConfig() const -> CtSortConfig override
      {
         return m_sort_config;
      }

      /// @brief specifies a new sort option, triggers DatasetEvent::Sort
      ///
      void applySortConfig(const CtSortConfig& config) override
      {
         if (config != m_sort_config)
         {
            m_sort_config = config;
            sortData();
         }
      }

      /// @brief retrieves a list of available filters for this table.
      ///
      auto availableStringFilters() const -> CtStringFilters override
      {
         return CtStringFilters{ std::from_range, StringFilters };
      }

      /// @brief get a list of values that can be used to filter on a column in the table
      ///
      auto getFilterMatchValues(int prop_idx) const -> StringSet override
      {
         return PropStringFilterMgr::getFilterMatchValues(m_data, Record::Traits::propFromIndex(prop_idx));
      }

      /// @brief adds a match value filter for the specified column.
      ///
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      /// 
      /// a record must match at least one match_value for each property that 
      /// has a filter to be considered a match.
      /// 
      auto addPropFilterString(int prop_idx, std::string_view match_value) -> bool override
      {
         // if we somehow get passed a filter we already have, don't waste our time.
         if ( m_prop_string_filters.addFilter(Record::Traits::propFromIndex(prop_idx), match_value) )
         {
            applyFilters();
            return true;
         }
         return false;
      }

      /// @brief removes a match value filter for the specified column.
      ///
      /// @return true if the filter was removed, false if it wasn't found
      /// 
      auto removePropFilterString(int prop_idx, std::string_view match_value) -> bool override
      {
         // if we somehow get passed filter that we aren't using, don't waste our time.
         if ( m_prop_string_filters.removeFilter(Record::Traits::propFromIndex(prop_idx), match_value) )
         {
            applyFilters();
            return true;
         }
         return false;
      }

      /// @brief filter does substring matching on ANY column in the table view
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers DatasetEvent::SubStringFilter
      ///
      auto filterBySubstring(std::string_view substr) -> bool override
      {
         // this overload searches all columns in the current grid, so get the prop_id's 
         auto cols = getDisplayColumns() | vws::transform([](const DisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
                                         | rng::to<std::vector>();

         return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
      }

      /// @brief sets filter that does substring matching on the specified column
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers DatasetEvent::SubStringFilter
      /// 
      auto filterBySubstring(std::string_view substr, int col_idx) -> bool override
      {
         auto prop = Traits::propFromIndex(col_idx);
         auto cols = std::vector{ prop };

         return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
      }

      /// @brief clear the substring filter
      ///
      /// triggers DatasetEvent::SubStringFilter
      /// 
      void clearSubStringFilter() override
      {
         m_substring_filter = std::nullopt;
         applyFilters();
      }

      /// @brief apples "in-stock only" filter to the data set, if supported.
      /// @return true if the filter was applied, false if it was not 
      /// 
      /// not all grid table support this filter, you can check by calling hasInStockFilter()
      /// 
      auto enableInStockFilter(bool enable) -> bool override
      {
         if (!hasInStockFilter()) 
            return false;

         if (enable == m_instock_filter.enabled)  
            return true;

         m_instock_filter.enabled = enable;
         applyFilters();
         return true;
      }

      /// @brief indicates whether the grid table supports filtering to in-stock only 
      /// @return true if supported, false otherwise.
      /// 
      constexpr auto hasInStockFilter() const -> bool override
      {  
         return Traits::supportsInStockFilter();  
      }

      /// @brief retrieves the minimum score filter value if active.
      /// 
      auto getMinScoreFilter() const -> NullableDouble override
      {
         if (m_score_filter.enabled)
         {
            return m_score_filter.compare_val.asDouble();
         }
         return std::nullopt;
      }

      /// @brief set the minimum score filter
      ///
      auto setMinScoreFilter(NullableDouble min_score = std::nullopt) -> bool override
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

      /// @brief Retrieve a property from the underlying table record (as opposed to a grid column)
      /// 
      /// Since we don't have table-neutral indices to use, this lookup has to be done by
      /// property name as a string that corresponds to correct enum. 
      /// 
      /// @return the property value formatted as a string if found, std::nullopt if not found 
      ///
      auto getDetailProp(int row_idx, std::string_view prop_name) const -> const CtProperty& override
      {
         auto maybe_prop = magic_enum::enum_cast<PropId>(prop_name);
         if (!maybe_prop)
            return null_prop; // can't return default-constructed because it would be ref to temp

         return (*m_current_view)[static_cast<size_t>(row_idx)][*maybe_prop];
      }

      /// @brief getTableName()
      /// @return the name of the CT table this grid table represents. Not meant to be 
      ///         displayed to the user, this is for internal use. 
      /// 
      auto getTableName() const -> std::string_view override
      {
         return Traits::getTableName();
      }

      void GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const override
      {
         // TODO
      }

      bool SetValueByRow(const wxVariant& variant, unsigned row, unsigned col) override
      {
         return false; // not supported
      }

      int totalRowCount() const override
      {
         return std::ssize(m_data);
      }

      int filteredRowCount() const override
      {
         return std::ssize(*m_current_view);
      }

      // default dtor, others are deleted since this object is meant to be heap-only
      ~CellarTrackerDataModel() noexcept override = default;
      CellarTrackerDataModel() = delete;
      CellarTrackerDataModel(const CellarTrackerDataModel&) = delete;
      CellarTrackerDataModel(CellarTrackerDataModel&&) = delete;
      CellarTrackerDataModel& operator=(const CellarTrackerDataModel&) = delete;
      CellarTrackerDataModel& operator=(CellarTrackerDataModel&&) = delete;

   private:
      DisplayColumns                   m_display_columns{};
      Dataset*                         m_current_view{};         // may point to m_data or m_filtered_data depending if filter is active
      Dataset                          m_data{};                 // the underlying data records for this table.
      Dataset                          m_filtered_data{};        // we need a copy for the filtered data, so we can bind our views to it
      PropFilter                       m_instock_filter{};
      PropFilter                       m_score_filter{};
      PropStringFilterMgr              m_prop_string_filters{};
      CtSortConfig                     m_sort_config{};
      std::optional<SubStringFilter>   m_substring_filter{};
      
      // private construction, use static factory method create();
      explicit CellarTrackerDataModel(Dataset data) : m_data{ std::move(data) }
      {}

      void applyFilters()
      {
         if (m_prop_string_filters.activeFilters() or m_instock_filter.enabled or m_score_filter.enabled)
         {
            m_filtered_data = vws::all(m_data) | vws::filter(m_prop_string_filters)
               | vws::filter(m_instock_filter)
               | vws::filter(m_score_filter)
               | rng::to<Dataset>();
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

      bool applySubStringFilter(const SubStringFilter& filter)
      {
         // clear any existing substring filter first, only one at a time. The new filter will be 
         // applied if there are any matches. If no matches, substring filter will be cleared
         // since we dont' restore it (by design, previous search text no longer in the toolbar
         // so it wouldn't make sense).
         m_substring_filter = {};
         applyFilters();
         auto filtered = vws::all(*m_current_view) | vws::filter(filter)
            | rng::to<Dataset>();
         if (filtered.empty())
            return false;

         m_substring_filter = filter;
         m_filtered_data.swap(filtered);
         m_current_view = &m_filtered_data;
         return true;
      }
      
      void sortData()
      {
         // sort the data table, then re-apply any filters to the view. Otherwise we'd have to sort twice
         if (m_sort_config.ascending)
         {
            rng::sort(m_data, Sorters[static_cast<size_t>(m_sort_config.sorter_index)]);
         }
         else {
            rng::sort(vws::reverse(m_data), Sorters[static_cast<size_t>(m_sort_config.sorter_index)]);
         }

         applyFilters();
         if (m_substring_filter)
         {
            applySubStringFilter(*m_substring_filter);
         }
      }

      bool isFilterActive() 
      { 
         return m_current_view = &m_filtered_data; 
      }

      //template<typename DatasetT>
      //CellarTrackerDataModel::create(DatasetT) -> CellarTrackerDataModel<DatasetT>;

      // Inherited via IDataset
};

} // namespace ctb::app