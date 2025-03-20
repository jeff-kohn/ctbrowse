#pragma once

#include "ctb/ctb.h"
#include <algorithm>
#include <functional>
#include <vector>


namespace ctb
{

   /// @brief template class that implements filter logic allowing to match a value to at least one specified property.
   ///
   /// note that there is no type coercion if ValueT is a variant. Comparing variants that aren't holding the
   /// same type will always evaluate to false.
   /// 
   /// while this class could technically be used for string filtering, it's not really designed for that
   /// and performance would be poor. There is a different class optimized for that use case (PropStringFilter)
   /// 
   template <CtRecord RecordTypeT, typename ValueT = typename RecordTypeT::TableProperty>
   struct PropFilter
   {
      using RecordType  = RecordTypeT;
      using PropId      = RecordType::PropId;
      using ValueType   = ValueT;
      using ComparePred = std::function<bool(const ValueType&, const ValueType&)>;


      /// @brief construct a PropFilter from any value convertible to ValueT for the specified property
      ///
      template<std::convertible_to<ValueType> T>
      constexpr PropFilter(PropId prop, ComparePred pred, T&& val) noexcept :
         match_props{ prop }, 
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(pred) },
         enabled{ true }
      {}


      /// @brief construct a PropFilter for matching the given value and predicate to one of multiple properties       
      ///
      template<std::convertible_to<ValueType> T> // requires std::same_as<rng::range_value_t<RngT>, PropId>
      constexpr PropFilter(std::initializer_list<PropId> props, ComparePred pred, T&& val) noexcept : 
         match_props(props.begin(), props.end()), 
         compare_val{ std::forward<T>(val) },
         compare_pred{ std::move(pred) },
         enabled{ true }
      {}


      /// @brief the properties that we're filtering against
      ///
      std::vector<PropId> match_props{};


      /// @brief the value the record property will be compared to using compare_pred
      ///
      ValueType compare_val{};


      /// @brief predicate that will be used to compare record properties to compare_val
      ///
      ComparePred compare_pred{ std::greater<ValueType>{} };


      /// @brief whether this fitler is active. If set to false, the operator() will always return true
      ///
      bool enabled{ false };


      /// @brief returns true if the specified table entry is a match to our predicate(s) and value
      ///
      bool operator()(const RecordType& rec) const
      {
         if (enabled)
            return rng::find_if(match_props, [&rec, this](PropId prop_id){ return compare_pred(rec[prop_id], compare_val); }) != match_props.end();

         return true;
      }


      PropFilter() noexcept = default;
      ~PropFilter() noexcept = default;
      PropFilter(const PropFilter&) = default;
      PropFilter(PropFilter&&) = default;
      PropFilter& operator=(const PropFilter&) = default;
      PropFilter& operator=(PropFilter&&) = default;
   };

} // namespace ctb