#pragma once
#include "ctb/ctb.h"
#include "ctb/tables/CtProperty.h"

#include <set>
#include <string_view>
#include <vector>


namespace ctb
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
      using Prop     = CtProp;
      using Property = CtProperty;

      /// @brief compile-time ctor, the only way to create an instance besides copy/assignment
      ///
      consteval CtStringFilter(const char* filter_name, Prop prop_id) :  m_filter_name(filter_name), m_prop_id(prop_id)
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

      /// @brief returns the zero-based index (into the table entry's CtProp enum) of the property this filter is for
      ///
      auto propId() const -> Prop
      {
         return m_prop_id;
      }

      /// @brief retrieve a list of available values in the table for this filter
      /// 
      template<typename DatasetT>
      auto getMatchValues(DatasetT* data) const -> StringSet
      {
         return data->getFilterMatchValues(m_prop_id);
      }

      /// @brief no default ctor or move semantics
      ///
      CtStringFilter() = delete;
      CtStringFilter(CtStringFilter&&) = delete;
      CtStringFilter& operator=(CtStringFilter&&) = delete;

   private:
      const char* m_filter_name{};
      Prop        m_prop_id{};
   };

   using CtStringFilters = std::vector<CtStringFilter>;

} // namespace ctb