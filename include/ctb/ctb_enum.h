#pragma once

#include <ctb/concepts.h>
#include <enchantum/enchantum.hpp>
#include <utility>

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

   using std::to_underlying;

};   // namespace ctb
