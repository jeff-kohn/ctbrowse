#include "App.h"
#include "CtDatasetOptions.h"
#include "json_serialization.h"

#include <ctb/model/CtDataset.h>
#include <glaze/glaze_exceptions.hpp>


namespace ctb::app
{
   namespace
   {
      auto getDefaultOptionsPath(TableId table_id)
      {
         return ctb::format("{}/{}.{}", wxGetApp().getDataFolder(AppFolder::Defaults).generic_string(), magic_enum::enum_name(table_id), "ctbc");
      }

   }


   auto CtDatasetOptions::applyOptionsToDataset(DatasetPtr dataset) const -> bool
   {
      using magic_enum::enum_name;

      if (nullptr == dataset)
         return false;

      bool all_good = true;

      auto failed = [&all_good](const std::string_view msg)
         {
            all_good = false;
            log::warn(msg);
            assert(false);
         };

      // warn if table-id doesn't match, but we can still try to apply other settings.
      if (table_id != dataset->getTableId())
      {
         auto expected_id = enum_name(table_id);
         auto actual_id = enum_name(dataset->getTableId());
         failed(ctb::format("Dataset Options for '{}' being applied to dataset '{}', this is probably a bug or an invalid options file.", expected_id, actual_id));
      }

      if (active_sort.sort_props.size() > 0 and dataset->hasProperty(active_sort.sort_props[0]))
      {
         dataset->applySort(active_sort);
      }
      else {
         failed(ctb::format("Dataset Options being applied to dataset '{}' contains invalid sort specification, this is probably a bug or an invalid options file.", enum_name(table_id)));
      }

      dataset->multivalFilters().assignFilters(multival_filters | 
         vws::transform([](auto&& filter)
            { 
               return std::make_pair(filter.prop_id, std::forward<decltype(filter)>(filter)); 
            }
         ));
      dataset->propFilters().assignFilters(prop_filters | 
         vws::transform([](auto&& filter)
            {  
               return std::make_pair(filter.filter_name, std::forward<decltype(filter)>(filter)); 
            }
         ));

      if (dataset->multivalFilters().size() < multival_filters.size() or dataset->propFilters().size() < prop_filters.size())
      {
         failed("One or more filters in the Dataset Options could not be applied to the Dataset");
      }

      return all_good;
   }


   auto CtDatasetOptions::loadOptionsFromDataset(DatasetPtr dataset) -> bool
   {
      if (nullptr == dataset)
      {
         assert("Passing nullptr dataset to this function never makes sense, this is a bug." and false);
         return false;
      }

      table_id         = dataset->getTableId();
      active_sort      = dataset->activeSort();
      multival_filters = dataset->multivalFilters().activeFilters() | vws::values | rng::to<std::vector>();
      prop_filters     = dataset->propFilters().activeFilters()     | vws::values | rng::to<std::vector>();

      return true;
   }


   /* static */ auto CtDatasetOptions::retrieveDefaultOptions(TableId table_id) -> std::optional<CtDatasetOptions>
   {
      try 
      {
         return retrieveOptions(getDefaultOptionsPath(table_id));
      }
      catch(...){ 
         log::exception(packageError());
         return {};
      }
   }


   /* static */ auto CtDatasetOptions::retrieveDefaultOptions(DatasetPtr dataset) -> CtDatasetOptions
   {
      auto table_id = dataset->getTableId();

      if (auto file_result = retrieveDefaultOptions(table_id); file_result.has_value())
      {
         return file_result.value();
      }
      
      return CtDatasetOptions{ .table_id = table_id, 
                               .active_sort = dataset->activeSort(), 
                               .multival_filters{ std::from_range, vws::values(dataset->multivalFilters().activeFilters()) },
                               .prop_filters{ std::from_range, vws::values(dataset->propFilters().activeFilters()) } };
   }

   /* static */ void CtDatasetOptions::applyDefaultOptions(DatasetPtr dataset)
   {
      auto result = retrieveDefaultOptions(dataset->getTableId());
      if (result)
      {
         result->applyOptionsToDataset(dataset);
      }
   }


   /* static */ auto CtDatasetOptions::retrieveOptions(const DatasetPtr dataset) noexcept(false) -> CtDatasetOptions
   {
      CtDatasetOptions result{};
      result.loadOptionsFromDataset(dataset);
      return result;
   }


   /* static */ auto CtDatasetOptions::retrieveOptions(const fs::path& path) noexcept(false) -> CtDatasetOptions
   {
      if (!fs::exists(path))
         throw Error{ ERROR_FILE_NOT_FOUND, Error::Category::FileError, constants::FMT_ERROR_FILE_NOT_FOUND, path.generic_string() };

      CtDatasetOptions result{};

      std::string buffer{};
      auto ec = glz::read_file_json(result, path.generic_string(), buffer);
      if (ec)
         throw Error{ glz::format_error(ec, buffer), Error::Category::ParseError };

      return result;
   }


   /* static */ void CtDatasetOptions::saveDefaultOptions(const CtDatasetOptions& options) noexcept(false)
   {
      saveOptions(options, getDefaultOptionsPath(options.table_id), true);
   }


   /* static */ void CtDatasetOptions::saveOptions(const CtDatasetOptions& options, const fs::path& json_path, bool overwrite) noexcept(false)
   {
      if (fs::exists(json_path) and !overwrite)
         throw Error{ ERROR_FILE_EXISTS, Error::Category::FileError, constants::FMT_ERROR_FILE_ALREADY_EXISTS, json_path.generic_string() };

      auto folder = json_path.parent_path();
      if (!fs::exists(folder))
      {
         fs::create_directories(folder);
      }

      constexpr auto glz_opts = glz::opts{ .prettify = true };
      auto ec = glz::write_file_json<glz_opts>(options, json_path.generic_string(), std::string{});
      if (ec)
         throw Error{ glz::format_error(ec), Error::Category::ParseError };
   }


} // namespace ctb::app