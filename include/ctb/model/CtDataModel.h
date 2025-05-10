#pragma once


#include "ctb/tables/detail/PropFilter.h"
#include "ctb/tables/detail/PropStringFilterMgr.h"
#include "ctb/tables/detail/SubStringFilter.h"
#include "ctb/tables/detail/TableSorter.h"

#include "ctb/interfaces/IDataset.h"
#include "ctb/model/DisplayColumn.h"

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
   class CtDataModel final : protected IDataset
   {
   public:
      using DataTable           = DataTableT;
      using Record              = DataTable::value_type;
      using Prop                = Record::Prop;
      using Property            = Record::Property;
      using Properties          = Record::Properties;
      using Traits              = Record::Traits;

      using DisplayColumn       = detail::DisplayColumn;
      using DisplayColumns      = detail::DisplayColumns;
      using PropFilter          = detail::PropFilter<Record, Property>;
      using PropStringFilterMgr = detail::PropStringFilterMgr<Record>;
      using SubStringFilter     = detail::SubStringFilter<Record>;
      using TableSort           = detail::TableSorter<Prop, Properties>;


      /// @brief list of display columns that will show in the list view
      ///
      static inline const std::array DefaultDisplayColumns { 
         DisplayColumn{ Prop::WineAndVintage,                              constants::DISPLAY_COL_WINE     },
         DisplayColumn{ Prop::Locale,                                      constants::DISPLAY_COL_LOCALE   },
         DisplayColumn{ Prop::QtyTotal,   DisplayColumn::Format::Number,   constants::DISPLAY_COL_QTY      },
         DisplayColumn{ Prop::CtScore,    DisplayColumn::Format::Decimal,  constants::DISPLAY_COL_CT_SCORE },
         DisplayColumn{ Prop::MyScore,    DisplayColumn::Format::Decimal,  constants::DISPLAY_COL_MY_SCORE },
      };

      /// @brief the available sort orders for this table.
      ///
      static inline const std::vector Sorters{ 
         TableSort{ { Prop::WineName, Prop::Vintage                       }, constants::SORT_OPTION_WINE_VINTAGE   },
         TableSort{ { Prop::Vintage,  Prop::WineName                      }, constants::SORT_OPTION_VINTAGE_WINE   },
         TableSort{ { Prop::Locale,   Prop::WineName,    Prop::Vintage    }, constants::SORT_OPTION_LOCALE_WINE    },
         TableSort{ { Prop::Region,   Prop::WineName,    Prop::Vintage    }, constants::SORT_OPTION_REGION_WINE    },
         TableSort{ { Prop::MyScore,  Prop::CtScore,     Prop::WineName,  }, constants::SORT_OPTION_SCORE_MY       },
         TableSort{ { Prop::CtScore,  Prop::MyScore,     Prop::WineName,  }, constants::SORT_OPTION_SCORE_CT       },
         TableSort{ { Prop::MyPrice,  Prop::WineName,    Prop::Vintage ,  }, constants::SORT_OPTION_MY_VALUE       }
      };

      /// @brief string filters that can be used on this table.
      ///
      static inline const std::array StringFilters{
         CtStringFilter{ constants::FILTER_VARIETAL,   Prop::Varietal       },
         CtStringFilter{ constants::FILTER_COUNTRY,    Prop::Country        },
         CtStringFilter{ constants::FILTER_REGION,     Prop::Region         },
         CtStringFilter{ constants::FILTER_APPELATION, Prop::Appellation    },
         CtStringFilter{ constants::FILTER_VINTAGE,    Prop::Vintage        }
      };

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
      /// 
      constexpr auto availableSorts() const -> const TableSorts& override
      {
         return Sorters;
      }

      /// @brief returns the currently active sort option
      ///
      auto activeSort() const -> const TableSort& override
      {
         return m_sort_config;
      }

      /// @brief specifies a new sort option
      ///
      void applySort(const TableSort& config) override
      {
         if (config != m_sort_config)
         {
            m_sort_config = config;
            sortData();
         }
      }

      /// @brief  get a list of the columns that will be displayed in the list
      /// 
      /// the columns are in the order they will be displayed.
      ///
      auto displayColumns() const -> const DisplayColumns& override
      { 
         return m_display_columns; 
      }

      /// @brief retrieves a list of available filters for this table.
      constexpr auto availableStringFilters() const -> CtStringFilters override
      {
         return CtStringFilters{ std::from_range, StringFilters };
      }

      /// @brief get a list of values that can be used to filter on a column in the table
      auto getFilterMatchValues(Prop prop_id) const -> StringSet override
      {
         return PropStringFilterMgr::getFilterMatchValues(m_data, prop_id);
      }

      /// @brief adds a match value filter for the specified column.
      ///
      /// @return true if the filter was applied, false it it wasn't because there were no matches
      /// 
      /// a record must match at least one match_value for each property that 
      /// has a filter to be considered a match.
      auto addPropFilterString(Prop prop_id, std::string_view match_value) -> bool override
      {
         // if we somehow get passed a filter we already have, don't waste our time.
         if ( m_prop_string_filters.addFilter(prop_id, match_value) )
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
      auto removePropFilterString(Prop prop_id, std::string_view match_value) -> bool override
      {
         // if we somehow get passed filter that we aren't using, don't waste our time.
         if ( m_prop_string_filters.removeFilter(prop_id, match_value) )
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
         // this overload searches all columns in the current list view, so get the prop_id's 
         auto cols = displayColumns() | vws::transform([](const DisplayColumn& disp_col) -> auto { return disp_col.prop_id; })
                                      | rng::to<std::vector>();

         return applySubStringFilter(SubStringFilter{ std::string{substr}, cols });
      }

      /// @brief sets filter that does substring matching on the specified column
      ///
      /// returns true if filter was applied, false if there were no matches in which
      /// case the filter was not applied. triggers DatasetEvent::SubStringFilter
      /// 
      auto filterBySubstring(std::string_view substr, Prop prop) -> bool override
      {
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

      /// @brief Returns whether "in-stock only" filter to the data set, if supported.
      /// @return true if the filter is active, false if it was not 
      /// 
      /// not all datasets support this filter, you can check by calling hasInStockFilter()
      /// 
      auto getInStockFilter() const -> bool override
      {
         if (!hasProperty(Prop::QtyOnHand) or !m_instock_filter.enabled)
            return false;

         return true;
      }

      /// @brief Used to enable/disable "in-stock only" filter to the data set, if supported.
      /// @return true if the filter was applied, false if it was not 
      /// 
      /// not all datasets support this filter, you can check by calling hasInStockFilter()
      /// 
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

      int64_t totalRecCount() const override
      {
         return std::ssize(m_data);
      }

      int64_t filteredRecCount() const override
      {
         return std::ssize(*m_current_view);
      }

      constexpr auto getTableName() const -> std::string_view override
      {
         return Traits::getTableName();
      }

      // default dtor, others are deleted since this object is meant to be heap-only
      ~CtDataModel() noexcept override = default;
      CtDataModel() = delete;
      CtDataModel(const CtDataModel&) = delete;
      CtDataModel(CtDataModel&&) = delete;
      CtDataModel& operator=(const CtDataModel&) = delete;
      CtDataModel& operator=(CtDataModel&&) = delete;


   private:
      DisplayColumns                   m_display_columns{};
      DataTable                        m_data{};                 // the underlying data records for this table.
      DataTable                        m_filtered_data{};        // we need a copy for the filtered data, so we can bind our views to it
      DataTable*                       m_current_view{};         // may point to m_data or m_filtered_data depending if filter is active
      PropFilter                       m_instock_filter{};
      PropFilter                       m_score_filter{};
      PropStringFilterMgr              m_prop_string_filters{};
      TableSort                        m_sort_config{};
      std::optional<SubStringFilter>   m_substring_filter{};
      
      // private construction, use static factory method create();
      explicit CtDataModel(DataTable&& data) : 
         m_display_columns{ std::from_range, DefaultDisplayColumns },
         m_data{ std::move(data) },
         m_current_view{ &m_data },
         m_instock_filter{ Prop::QtyOnHand, std::greater<CtProperty>{}, uint16_t{0} },
         m_score_filter{ {Prop::CtScore, Prop::MyScore}, std::greater_equal<CtProperty>{}, constants::FILTER_SCORE_DEFAULT },
         m_sort_config{ availableSorts()[0] }
      {
         m_score_filter.enabled = false;
         m_instock_filter.enabled = false;
         sortData();
      }

      void applyFilters()
      {
         throw Error{"FUCK"};
         //if (m_prop_string_filters.activeFilters() or m_instock_filter.enabled or m_score_filter.enabled)
         //{
         //   m_filtered_data = vws::all(m_data) | vws::filter(m_prop_string_filters)
         //                                      | vws::filter(m_instock_filter)
         //                                      | vws::filter(m_score_filter)
         //                                      | rng::to<decltype(m_data)>();
         //   m_current_view = &m_filtered_data;
         //}
         //else
         //{
         //   m_current_view = &m_data;
         //}

         //if (m_substring_filter)
         //{
         //   applySubStringFilter(*m_substring_filter);
         //}
      }

      bool applySubStringFilter(const SubStringFilter& filter)
      {
         throw ctb::Error{" FUCKITY FUCKING FUCK"};
         //// clear any existing substring filter first, since we can only one at a time. The 
         //// new filter will be applied if there are any matches. If no matches, substring filter 
         //// will be cleared since we don't restore it (by design, previous search text is no longer 
         //// in the toolbar so it wouldn't make sense).
         //m_substring_filter = {};
         //applyFilters();
         //auto filtered = vws::all(*m_current_view) | vws::filter(filter)
         //                                          | rng::to<std::vector<Record> >();
         //if (filtered.empty())
         //   return false;

         //m_substring_filter = filter;
         //m_filtered_data.swap(filtered);
         //m_current_view = &m_filtered_data;
         //return true;
      }
      
      void sortData()
      {
         throw Error{ "Not Implemented"};
         // sort the data table, then re-apply any filters to the view. Otherwise we'd have to sort twice
         //if (m_sort_config.descending)
         //{
         //   rng::sort(vws::reverse(m_data), Sorters[static_cast<size_t>(m_sort_config.sorter_index)]);
         //}
         //else {
         //   rng::sort(m_data, Sorters[static_cast<size_t>(m_sort_config.sorter_index)]);
         //}

         //applyFilters();
         //if (m_substring_filter)
         //{
         //   applySubStringFilter(*m_substring_filter);
         //}
      }

      bool isFilterActive() 
      { 
         return m_current_view == &m_filtered_data; 
      }

      auto hasProperty(Prop prop_id) const -> bool override
      {
         return false;
      }

      auto getProperty(int rec_idx, Prop prop_id) const -> const Property & override
      {
         if (rec_idx > filteredRecCount() or !hasProperty(prop_id))
            return null_prop;

         return m_current_view->at(static_cast<size_t>(rec_idx))[prop_id];
      }

};

} // namespace ctb