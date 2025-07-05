/*********************************************************************
* @file       PropertyFilter.h
*
* @brief      declaration for the PropertyFilter class
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#pragma once

#include "ctb/ctb.h"

#include <functional>


namespace ctb::detail
{
   /// @brief wraps a binary predicate so that it can be serialized, since std::function<> can't be directly serialized
   //
   template<PropertyValueType PropertyValT>
   class PropertyFilterPredicate
   {
   public:
      using PropertyVal = PropertyValT;
      using CompareFunction = std::function<bool(const PropertyVal&, const PropertyVal&)>;

      enum class PredicateType
      {
         Equal,
         Greater,
         GreaterEqual,
         Less,
         LessEqual
      };


      explicit PropertyFilterPredicate(PredicateType predicate_type)
      {
         setPredicateType(predicate_type);
      }

      /// @brief Returns the comparison type for this filter predicate
      PredicateType predicateType() const
      {
         return m_predicate_type;
      }

      /// @brief Sets the comparison type for this filter predicate
      void setPredicateType(const PredicateType& predicate_type)
      {
         m_predicate_type = predicate_type;  // Store the new predicate type

         switch (predicate_type)  // Use the input parameter
         {
            case PredicateType::Equal:
               m_compare_func = std::equal_to<PropertyVal>{};
               break;
            case PredicateType::Greater:
               m_compare_func = std::greater<PropertyVal>{};
               break;
            case PredicateType::GreaterEqual:
               m_compare_func = std::greater_equal<PropertyVal>{};
               break;
            case PredicateType::Less:
               m_compare_func = std::less<PropertyVal>{};
               break;
            case PredicateType::LessEqual:
               m_compare_func = std::less_equal<PropertyVal>{};
               break;
            default:
               assert(false);
         }
      }

      auto operator()(const PropertyVal& p1, const PropertyVal& p2) const -> bool
      {
         return m_compare_func(p1, p2);
      }

      PropertyFilterPredicate() = default;
      ~PropertyFilterPredicate() noexcept = default;
      PropertyFilterPredicate(const PropertyFilterPredicate&) = default;
      PropertyFilterPredicate(PropertyFilterPredicate&&) = default;
      PropertyFilterPredicate& operator=(const PropertyFilterPredicate&) = default;
      PropertyFilterPredicate& operator=(PropertyFilterPredicate&&) = default;

   private:
      PredicateType   m_predicate_type{ PredicateType::Equal };
      CompareFunction m_compare_func{};
   };

} // namespace ctb


