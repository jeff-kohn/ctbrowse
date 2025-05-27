#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/CtSchema.h"


namespace ctb::detail
{

   inline auto getValueOrNull(const CtPropertyMap& rec, CtProp prop_id) -> const CtProperty&
   {
      auto it = rec.find(prop_id);
      if (it == rec.end() )
         return ct_null_prop;  // null is better option than throwing

      return it->second;
   }


   inline auto getWineAndVintage(const CtPropertyMap& rec) -> CtProperty
   {
      auto vintage   = getValueOrNull(rec, CtProp::Vintage ).asString();
      auto wine_name = getValueOrNull(rec, CtProp::WineName).asStringView();
      return ctb::format("{} {}", vintage, wine_name);
   }


   inline auto getRtdConsumed(const CtPropertyMap& rec) -> CtProperty
   {
      auto purchased = getValueOrNull(rec, CtProp::QtyPurchased).asUInt16().value_or(0);
      auto consumed  = getValueOrNull(rec, CtProp::QtyConsumed ).asUInt16().value_or(0);
      auto pending   = getValueOrNull(rec, CtProp::QtyPending  ).asUInt16().value_or(0);

      if (consumed)
      {
         return ctb::format("{}-{}={}", purchased, consumed, purchased - consumed);
      }
      else if (purchased) 
      {
         return ctb::format("{}", purchased);
      }
      else {
         return ctb::format("({})", pending);
      }
   }


   inline auto calcQtyTotal(const CtPropertyMap& rec) -> CtProperty
   {
      auto qty     = getValueOrNull(rec, CtProp::QtyOnHand ).asUInt16().value_or(0u);
      auto pending = getValueOrNull(rec, CtProp::QtyPending).asUInt16().value_or(0u);

      CtProperty result{};
      if (pending == 0)
      {
         result = qty;
      }
      else if (qty == 0)
      {
         result = ctb::format("({})", pending);
      }
      else {
         result = ctb::format("{}+{}", qty, pending);
      }
      return result;
   }

   inline void validateDrinkYear(CtProperty& prop) 
   {
      if (prop.asUInt16() == constants::CT_NULL_YEAR)
         prop.setNull();
   }



} // namesapace ctb::detail
