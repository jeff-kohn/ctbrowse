/*********************************************************************
* @file       SortConfig.h
*
* @brief      defines the SortConfig struct
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#pragma once

#include <string_view>


namespace ctb::detail
{
   /// @brief Struct that contains name, index and direction of a sort config.
   ///        Used by DatasetBase class.
   ///
   struct SortConfig
   {
      int               sorter_index{}; // index of the sorter in a corresoponding Sorters collection
      std::string_view  sorter_name{};  // user-facing name of the sort option
      bool              descending{ false };

      [[nodiscard]] 
      auto operator<=>(const SortConfig&) const -> std::strong_ordering = default;
   };

} // namespace ctb::detail
