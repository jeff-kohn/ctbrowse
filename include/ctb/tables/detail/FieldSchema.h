
#pragma once

namespace ctb::detail
{
   /// @brief enum to specify the data formats a property value can contain.
   /// 
   /// these enums indicate how to attempt to parse/interpret the CSV field for a given property.
   ///
   enum class PropType
   {
      String,
      UInt16,
      UInt64,
      Double,
      Date
   };

   /// @brief  contains the property type and CSV column index for a given Prop
   template<typename PropT> requires std::is_enum_v<PropT>
   struct FieldSchema
   {
      using Prop = PropT;

      Prop           prop_id{};
      PropType       prop_type{};
      NullableShort  csv_col{};   // will be std::nullopt for custom fields not in the CSV
   };

}