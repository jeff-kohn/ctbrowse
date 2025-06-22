#pragma once

#include "App.h"
#include <ctb/table_data.h>
#include <ctb/tables/CtSchema.h>

namespace ctb::app
{
   /// @brief This struct represents the various sort/filter options that can be applied to a dataset,
   ///        and can be persisted to and later loaded from disk or elsewhere to save/load settings for a collection/view.
   struct CtDatasetOptions
   {
      /// @brief TableId enum representing the dataset this collection models. Since we'll serialize to json
      ///        and it's conceivable that the json string isn't parsable to enum, use a safe default
      TableId table_id{ TableId::List };

      /// @brief vector of multi-val filters to apply to the dataset
      std::vector<CtMultiValueFilter> multi_val_filters{};

      /// @brief vector of property filters to apply to the dataset
      std::vector<CtPropertyFilter>   m_prop_filters{};

      /// @brief Sort settings to apply to the dataset
      CtTableSort active_sort{};
   };


} // namespace ctb::app