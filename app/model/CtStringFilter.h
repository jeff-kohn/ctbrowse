#pragma once

#include "App.h"

#include <string_view>
#include <set>
#include <vector>


namespace ctb::app
{
   /// @brief  class that contains a filter specification
   ///
   /// this class only has consteval initialization since it takes a string literal as a param
   /// and this way we don't have to worry about it being invalidated since it's by definition
   /// in static storage.
   /// 
   class CtStringFilter
   {
   public:
      /// @brief compile-time ctor, the only way to create an instance besides copy/assignment
      ///
      consteval CtStringFilter(const char* filter_name, int prop_index) : 
         m_filter_name(filter_name),
         m_prop_index(prop_index)
      {}
      constexpr CtStringFilter(const CtStringFilter&) = default;
      constexpr CtStringFilter& operator=(const CtStringFilter&) = default;
      ~CtStringFilter() = default;

      /// @brief returns the name/description of this filter
      /// 
      auto filterName() const -> std::string_view
      {
         return m_filter_name;
      }

      /// @brief returns the zero-based index (into the table entry's PropId enum) of the property this filter is for
      ///
      auto propIndex() const -> int
      {
         return m_prop_index;
      }

      /// @brief retrieve a list of available values in the table for this filter
      /// 
      template<typename DatasetT>
      auto getMatchValues(DatasetT* data) const -> StringSet
      {
         return data->getFilterMatchValues(m_prop_index);
      }

      /// @brief no default ctor or move semantics
      ///
      CtStringFilter() = delete;
      CtStringFilter(CtStringFilter&&) = delete;
      CtStringFilter& operator=(CtStringFilter&&) = delete;

   private:
      const char* m_filter_name{};
      int         m_prop_index{};
   };

   using CtStringFilters = std::vector<CtStringFilter>;

} // namespace ctb::app