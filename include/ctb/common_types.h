/*******************************************************************
 * @file common_types.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/TableProperty.h"

#include <string>
#include <optional>
#include <set>


// these are just some type aliases used throughout the application, that I don't want to
// have to keep redeclaring.
//
namespace ctb
{
   using NullableShort  = std::optional<uint16_t>;
   using NullableInt    = std::optional<int32_t>;
   using NullableDouble = std::optional<double>;

   using StringSet = std::set<std::string, std::less<>>;

   /// @brief this is the common property object used by all of our tables. 
   using CtProperty = TableProperty<uint16_t, uint64_t, double, std::string>;

   /// @brief useful if you need to return a const ref to a null property value
   static inline constexpr CtProperty null_prop{};
}