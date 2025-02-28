#pragma once

#include "ctb/ctb.h"

namespace ctb
{


   /// @brief template class that implements a predicate functor for our recordtype classes, allowing comparison
   ///
   ///
   template <CtRecord RecordTypeT, typename ValueT = CtProperty, std::predicate<ValueT, ValueT> ComparePredT = std::greater<>  >
   struct PropertyFilterNumeric 
   {
      using RecordType  = RecordTypeT;
      using PropId      = RecordType::PropId;
      using Value       = ValueT;
      using ComparePred = ComparePredT;


      /// @brief construct a PropertyFilterNumeric from any value convertible to CompareVal for the specified property
      ///
      template<std::convertible_to<Value> T>
      constexpr PropertyFilterNumeric(PropId prop, T&& val) noexcept : prop_id(prop), compare_val{ std::forward<T>(val) }
      {}


      /// @brief the property that we're filtering against
      ///
      PropId prop_id{};


      /// @brief the value this will compared to using ComparedPred
      Value compare_val{};


      /// @brief the binary predicate that will be used.
      ComparePred compare_pred{};


      /// @brief returns true if the specified table entry is a match to our predicate
      ///
      bool operator()(const RecordType& rec) const
      {
         return compare_pred(rec[prop_id], compare_val);
      }
      

      PropertyFilterNumeric() noexcept = default;
      PropertyFilterNumeric(const PropertyFilterNumeric&) = default;
      PropertyFilterNumeric(PropertyFilterNumeric&&) = default;
      PropertyFilterNumeric& operator=(const PropertyFilterNumeric&) = default;
      PropertyFilterNumeric& operator=(PropertyFilterNumeric&&) = default;
      ~PropertyFilterNumeric() noexcept = default;
   };

} // namespace ctb