﻿/*******************************************************************
 * @file common_types.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include <string>
#include <optional>
#include <set>
#include <span>
#include <vector>

// these are just some type aliases used throughout the application, that I don't want to
// have to keep redeclaring.
//
namespace ctb
{
   using NullableShort  = std::optional<uint16_t>;
   using NullableInt    = std::optional<int32_t>;
   using NullableDouble = std::optional<double>;

   using MaybeString    = std::optional<std::string>;
   using StringSet      = std::set<std::string, std::less<>>;

   using Buffer         = std::vector<std::byte>;
   using BufferSpan     = std::span<std::byte>;
}