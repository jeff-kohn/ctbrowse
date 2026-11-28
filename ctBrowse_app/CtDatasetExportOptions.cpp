#include "CtDatasetExportOptions.h"
#include <ctb/json_serialization.h>

namespace ctb::app
{
   auto getExportOptionsForDataset(const DatasetPtr &ds) -> CtDatasetExportOptions
   {
      if (!ds) return {};

      auto collection_name = ds->getCollectionName();

      // this is a what we'll use if we can't successfully load a compatible export option set from the config store.
      CtDatasetExportOptions ds_options{ .table_id = ds->getTableId(), .collection_name = collection_name };

      auto saved_result = getExportOptionsForCollection(collection_name);
      if (!saved_result)
      {
         log::warn(saved_result.error().formattedMessage());

         return ds_options;
      }

      if (ds->getTableId() != saved_result->table_id)
      {
         log::warn("Saved export settings for collection {} are not a match for table {}, using table defaults",
                   collection_name,
                   enum_to_string(ds->getTableId()));

         return ds_options;
      }

      return std::move(saved_result.value());
   }


   auto getExportOptionsForCollection(std::string_view collection_name) -> std::expected<CtDatasetExportOptions, ctb::Error>
   {

      ScopedConfigPath config{ wxGetApp().getConfig(ctb::format(constants::FMT_CONFIG_PATH_DATASET_EXPORT, collection_name)) };
      auto             serialized_options = config->Read(constants::CONFIG_VAL_EXPORT_OPTIONS, wxEmptyString).utf8_string();

      if (serialized_options.empty())
         return std::unexpected{ ctb::Error{ Error::Category::GeneralError, constants::FMT_ERROR_STR_NO_EXPORT_OPTIONS, collection_name } };

      auto saved_options = CtDatasetExportOptions{};
      auto ec            = glz::read_json(saved_options, serialized_options);
      if (ec)
      {
         return std::unexpected{
            ctb::Error{ Error::Category::GeneralError,
                       constants::FMT_ERROR_STR_INVALID_EXPORT_OPTIONS,
                       collection_name,
                       glz::format_error(ec) }
         };
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

      ScopedConfigPath config{ wxGetApp().getConfig(
         ctb::format(constants::FMT_CONFIG_PATH_DATASET_EXPORT, export_options.collection_name.c_str())) };
      config->Write(constants::CONFIG_VAL_EXPORT_OPTIONS, serialized_result->c_str());
   }

}   // namespace ctb::app

