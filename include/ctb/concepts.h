/*********************************************************************
 * @file       concepts.h
 *
 * @brief      defines some app-specific concepts
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include <concepts>
#include <ranges>
#include <string_view>

namespace ctb
{
   namespace rng = std::ranges;
   namespace vws = rng::views;

   /// @brief concept for a type that is convertible to std::string_view
   template <typename T>
   concept StringViewCompatible = std::convertible_to<T, std::string_view>;


   /// @brief concept for a traits object definining the schema for a CtRecord instantiation
   ///
   template <typename T> 
   concept CtRecordTraits = requires (T t, typename T::PropId p)
   {
      t.getCsvSchema();
   };


   /// @brief concept for a record object representing a row in a CT table (CSV file)
   ///
   template <typename T> 
   concept CtRecord = requires (T t, typename T::PropId p, typename T::RowType r)
   {
     t.parse(r);
     t.getProperty(p);
   };


   /// @brief concept for a type that implements the interface of std::optional
   ///
   template<typename T> 
   concept Nullable = requires (T t, T::value_type v1, T::value_type v2)
   {
      v1 = t ? *t : v2;
      v1 = t.has_value() ? t.value() : v2;
   };

   /// @brief concept for a type that is either integral or floating point.
   ///
   template <typename T>
   concept Arithmetic = std::integral<T> or std::floating_point<T>; 


} // namespace ctb
