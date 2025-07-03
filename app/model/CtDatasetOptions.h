#pragma once

#include "App.h"

#include <ctb/interfaces/IDataset.h>
#include <ctb/table_data.h>
#include <ctb/tables/CtSchema.h>

#include <optional>

namespace ctb::app
{
   /// @brief This struct represents the various sort/filter options that can be applied to a dataset,
   ///        and can be persisted to and later loaded from disk or elsewhere to save/load settings for a collection/view.
   struct CtDatasetOptions
   {
      /// @brief TableId enum representing the dataset this collection models. Since we'll serialize to json
      ///        and it's conceivable that the json string isn't parseable to enum, use a safe default
      TableId table_id{ TableId::List };

      /// @brief Sort settings to apply to the dataset
      CtTableSort active_sort{};

      /// @brief vector of multi-val filters to apply to the dataset
      std::vector<CtMultiValueFilter> multival_filters{};

      /// @brief vector of property filters to apply to the dataset
      std::vector<CtPropertyFilter>   prop_filters{};

      /// @brief Apply options from this object to the provided Dataset
      /// @return true if all options were successfully applied, false if one or more could not be applied.
      auto applyOptionsToDataset(DatasetPtr dataset) const -> bool;

      /// @brief Load current options from the provided dataset to this object
      /// @return true if options were loaded, false if they weren't (because dataset == nullptr, you dummy)
      auto loadOptionsFromDataset(const DatasetPtr dataset) -> bool;

      /// @brief Apply any saved default CtDatasetOptions to the supplied dataset.
      ///
      /// If no saved default is found, dataset will not be modified.
      static void applyDefaultOptions(DatasetPtr dataset);

      /// @brief Retrieve a CtDatasetOptions initialized from the supplied dataset.
      /// @throw ctb::Error if you pass a nullptr dataset
      static auto retrieveOptions(const DatasetPtr dataset) noexcept(false) -> CtDatasetOptions;

      /// @brief Retrieve a CtDatasetOptions initialized from the specified json file
      /// @throw ctb::Error if file can't be read and loaded into object
      static auto retrieveOptions(const fs::path& json_path) noexcept(false) -> CtDatasetOptions;

      /// @brief Retrieve a CtDatasetOptions with default options for the specified TableId, if it exists
      /// @return the requested options object, or std::nullopt if no default was found.
      static auto retrieveDefaultOptions(TableId table_id) -> std::optional<CtDatasetOptions>;

      /// @brief Retrieve a CtDatasetOptions with default options for the specified dataset.
      ///
      /// If a saved CtDatasetOptions is found, it will be returned to the caller. If no saved default
      /// is found, the supplied dataset's current settings will be returned.
      static auto retrieveDefaultOptions(DatasetPtr dataset) -> CtDatasetOptions;


      /// @brief Saves the provided object as the new default for its TableId
      /// @throw ctb::Error if the options object can't be saved to a file.
      static void saveDefaultOptions(const CtDatasetOptions& options) noexcept(false);

      /// @brief Save a CtDatasetOptions object to the specified json file
      /// @throw ctb::Error if saving file fails
      static void saveOptions(const CtDatasetOptions& options, const fs::path& json_path, bool overwrite) noexcept(false);

   };




} // namespace ctb::app