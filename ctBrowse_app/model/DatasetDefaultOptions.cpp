#include "DatasetDefaultOptions.h"

namespace ctb::app::DatasetDefaultOptions
{

   namespace
   {
      auto getDefaultOptionsPath(TableId table_id)
      {
         // At the moment this is the only dependency preventing this class from being moved to the lib in
         // the future if we decide that's desirable.
         return ctb::format(
            "{}/{}.{}", wxGetApp().getDataFolder(AppFolder::Defaults).generic_string(), getTableDescription(table_id), "ctbc");
      }
   }   // namespace


   auto retrieveDefaultOptions(TableId table_id) -> std::optional<CtDatasetOptions>
   {
      try
      {
         auto default_path = getDefaultOptionsPath(table_id);
         if (fs::exists(default_path))
         {
            return CtDatasetOptions::retrieveOptions(default_path);
         }
      }
      catch (...)
      {
         log::info(
            "Saved default for Dataset '{}' could not be loaded ({}).", getTableDescription(table_id), packageError().formattedMessage());
      }
      return {};
   }


   auto retrieveDefaultOptions(const DatasetPtr& dataset) -> CtDatasetOptions
   {
      auto table_id = dataset->getTableId();

      if (auto file_result = retrieveDefaultOptions(table_id); file_result.has_value())
      {
         return file_result.value();
      }

      return CtDatasetOptions{
         .table_id        = table_id,
         .collection_name = dataset->getCollectionName(),
         .active_sort     = dataset->activeSort(),
         .multival_filters{ std::from_range, vws::values(dataset->multivalFilters().activeFilters()) },
         .prop_filters{ std::from_range, vws::values(dataset->propFilters().activeFilters())     }
      };
   }


   void applyDefaultOptions(DatasetPtr& dataset)
   {
      auto result = retrieveDefaultOptions(dataset->getTableId());
      if (result)
      {
         result->applyToDataset(dataset);
      }
   }

   
   void saveDefaultOptions(const CtDatasetOptions& options) noexcept(false)
   {
      CtDatasetOptions::saveOptions(options, getDefaultOptionsPath(options.table_id), true);
   }


}   // namespace ctb::app
