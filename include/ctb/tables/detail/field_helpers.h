#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/CtSchema.h"


namespace ctb::detail
{

   /// @brief Retrieve a property from a record's property map. 
   /// @return const reference to the requested property, or a null property if requested property is not found.
   inline auto getValueOrNull(const CtPropertyMap& rec, CtProp prop_id) -> const CtPropertyVal&
   {
      auto it = rec.find(prop_id);
      if (it == rec.end() )
         return ct_null_prop;  // null is better option than throwing

      return it->second;
   }

   /// @brief Get the wine and vintage together as a combine string.
   inline auto getWineAndVintage(const CtPropertyMap& rec) -> CtPropertyVal
   {
      auto vintage   = getValueOrNull(rec, CtProp::Vintage ).asString();
      auto wine_name = getValueOrNull(rec, CtProp::WineName).asStringView();
      return ctb::format("{} {}", vintage, wine_name);
   }

   /// @brief Get formatted display of drinkable inventory (purchased - consumed + pending)
   /// @return string in the form of "Total-Drunk=Remaining", "Total", or "(Pending)"
   inline auto getRtdInventory(const CtPropertyMap& rec) -> CtPropertyVal
   {
      auto purchased = getValueOrNull(rec, CtProp::QtyPurchased).asUInt16().value_or(0);
      auto consumed  = getValueOrNull(rec, CtProp::QtyConsumed ).asUInt16().value_or(0);
      auto pending   = getValueOrNull(rec, CtProp::QtyPending  ).asUInt16().value_or(0);
      auto remaining = purchased + pending - consumed;

      CtPropertyVal result{};
      if (consumed)
      {
         if (pending)
            result = ctb::format("{}-{}+({})={}", purchased, consumed, pending, remaining);
         else
            result = ctb::format("{}-{}={}", purchased, consumed, remaining);
      }
      else if (purchased) 
      {
         if (pending)
            result = ctb::format("{}+({})={}", purchased, pending, remaining);
         else
            result = ctb::format("{}", purchased);
      }
      else {
         result = ctb::format("({})", pending);
      }
      return result;
   }

   /// @brief  Get total quantity as formatted string
   /// @return string in format  "1" (in-stock only), "1+(1)" (in-stock + pending) or "(1)" (pending only)
   inline auto calcQtyTotal(const CtPropertyMap& rec) -> CtPropertyVal
   {
      auto qty     = getValueOrNull(rec, CtProp::QtyOnHand ).asUInt16().value_or(0u);
      auto pending = getValueOrNull(rec, CtProp::QtyPending).asUInt16().value_or(0u);

      CtPropertyVal result{};
      if (qty == 0)
      {
         result = ctb::format("({})", pending);
      }
      else if (pending == 0)
      {
         result = qty;
      }
      else {
         result = ctb::format("{}+({})", qty, pending);
      }
      return result;
   }

   /// @brief  Replace drink date of 9999 with null
   inline void validateDrinkYear(CtPropertyVal& prop) 
   {
      if (prop.asUInt16() == constants::CT_NULL_YEAR)
         prop.setNull();
   }



} // namesapace ctb::detail
