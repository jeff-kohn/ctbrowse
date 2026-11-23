#include "CtDatasetExportOptions.h"
#include <ctb/json_serialization.h>

namespace ctb::app
{
   auto getDefaultExportOptions(const DatasetPtr &ds) -> CtDatasetExportOptions
   {
      if (!ds) return {};

      auto collection_name = ds->getCollectionName();

      // this is a what we'll use if we can't successfully load a compatible export option set from the config store.
      CtDatasetExportOptions ds_options{ .table_id = ds->getTableId(), .collection_name = collection_name };

      ScopedConfigPath config{ wxGetApp().getConfig(ctb::format(constants::FMT_CONFIG_PATH_DATASET_EXPORT, ds_options.collection_name)) };
      auto             serialized_options = config->Read(constants::CONFIG_VAL_EXPORT_OPTIONS, wxEmptyString).utf8_string();

      if (serialized_options.empty()) return ds_options;

      auto saved_options = CtDatasetExportOptions{};
      auto ec            = glz::read_json(saved_options, serialized_options);
      if (ec)
      {
         log::error(
            "Couldn't deserialize export export_options for collection '{}' from the config store. {}", collection_name, glz::format_error(ec));
         assert(false);
         return ds_options;
      }
      if (ds->getTableId() != saved_options.table_id)
      {
         log::error("Saved default export export_options for collection '{}' are not compatible with the supplied dataset. {} != {}.",
                    collection_name, ds_options.table_id, saved_options.table_id);
         assert(false);
         return ds_options;
      }
      return saved_options;
   }


   void saveDefaultExportOptions(const CtDatasetExportOptions &export_options) noexcept(false)
   {
      auto serialized_result = glz::write_json(export_options);
      if (!serialized_result)
      {
         throw ctb::Error{ Error::Category::ParseError, constants::FMT_EXPORT_OPTIONS_SERIALIZE_ERROR,
                           glz::format_error(serialized_result.error()) };
      }

      ScopedConfigPath config{ wxGetApp().getConfig(ctb::format(constants::FMT_CONFIG_PATH_DATASET_EXPORT, export_options.collection_name.c_str())) };
      config->Write(constants::CONFIG_VAL_EXPORT_OPTIONS, serialized_result->c_str());
      
   }


   void runAutoExportJobs(const fs::path &collection_folder, bool overwrite) noexcept
   {
      if (!fs::exists(collection_folder)) return;

      // For each saved collection file in the specified folder, load the dataset and then run an export to file.
   }
}   // namespace ctb::app

