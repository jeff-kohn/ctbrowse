#pragma once

#include "App.h"

#include <string_view>
#include <set>


namespace ctb::app
{
   class GridTable;

   /// type alias
   using StringSet = std::set<std::string, std::less<> >;


   /// @brief  class that contains a filter specification
   ///
   /// this class only has consteval initialization since it takes a string literal as a param
   /// and this way we don't have to worry about it being invalidated since it's by definition
   /// in static storage.
   /// 
   /// if you need delayed initialization you'll have to use a heap-allocated instance that is
   /// copy-initialized.
   /// 
   class GridTableFilter
   {
   public:
      /// @brief compile-time ctor, the only way to create an instance besides copy/assignment
      ///
      consteval GridTableFilter(const char* filter_name, int prop_index) : 
         m_filter_name(filter_name),
         m_prop_index(prop_index)
      {}
      constexpr GridTableFilter(const GridTableFilter&) = default;
      constexpr GridTableFilter& operator=(const GridTableFilter&) = default;
      ~GridTableFilter() = default;


      /// @brief returns the name/description of this filter
      /// 
      std::string_view filterName() const
      {
         return m_filter_name;
      }


      /// @brief returns the index (into the table entry's PropId enum) of the property this filter is for
      ///
      int propIndex() const 
      {
         return m_prop_index;
      }


      /// @brief retrieve a list of available values in the table for this filter
      /// 
      StringSet getMatchValues(GridTable* grid_table) const;


      /// @brief no default ctor or move semantics
      ///
      GridTableFilter() = delete;
      GridTableFilter(GridTableFilter&&) = delete;
      GridTableFilter& operator=(GridTableFilter&&) = delete;

   private:
      const char* m_filter_name{};
      int         m_prop_index{};


   };

} // namespace ctb::app