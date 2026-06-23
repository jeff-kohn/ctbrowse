#pragma once

#include "App.h"

#include <ctb/model/CtDatasetOptions.h>


namespace ctb::app::DatasetDefaultOptions
{

   /// @brief Retrieve a CtDatasetOptions with default options for the specified TableId, if it exists
   /// @return the requested options object, or std::nullopt if no default was found.
   [[nodiscard]] auto retrieveDefaultOptions(TableId table_id) -> std::optional<CtDatasetOptions>;

   /// @brief Retrieve a CtDatasetOptions with default options for the specified dataset.
   ///
   /// If a saved CtDatasetOptions is found, it will be returned to the caller. If no saved default
   /// is found, the supplied dataset's current settings will be returned.
   [[nodiscard]] auto retrieveDefaultOptions(const DatasetPtr& dataset) -> CtDatasetOptions;


   /// @brief Saves the provided object as the new default for its TableId
   /// @throw ctb::Error if the options object can't be saved to a file.
   [[nodiscard]] void saveDefaultOptions(const CtDatasetOptions& options) noexcept(false);


   /// @brief Apply default options to a dataset
   [[nodiscard]] void applyDefaultOptions(DatasetPtr& dataset);


}   // namespace ctb::app::DatasetDefaultOptions
