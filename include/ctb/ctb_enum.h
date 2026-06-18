#pragma once

#include "ctb/ctb.h"
#include <enchantum/enchantum.hpp>

namespace ctb
{
   static constexpr auto& enum_to_string = enchantum::to_string;

   template<EnumType EnumT>
   consteval auto enum_count()
   {
      return enchantum::count<EnumT>;
   }

   template<EnumType EnumT, IntegralType NumT>
   constexpr auto enum_cast(NumT num)
   {
      return enchantum::cast<EnumT>(num);
   }

   using enchantum::enum_to_index;
   using enchantum::index_to_enum;
};   // namespace ctb
