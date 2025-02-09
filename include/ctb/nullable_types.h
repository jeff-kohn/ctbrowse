/*******************************************************************
 * @file nullable_types.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include <optional>

namespace ctb
{
   using NullableShort = std::optional<uint16_t>;
   using NullableInt = std::optional<int32_t>;
   using NullableDouble = std::optional<double>;
}