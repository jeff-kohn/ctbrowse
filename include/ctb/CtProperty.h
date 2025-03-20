#pragma once
#include "ctb/TableProperty.h"

namespace ctb
{

   /// @brief this is the common property object used by all of our tables. 
   using CtProperty = TableProperty<uint16_t, uint64_t, double, std::string>;

   /// @brief useful if you need to return a const ref to a null property value
   static inline constexpr CtProperty null_prop{};


} // namespace ctb