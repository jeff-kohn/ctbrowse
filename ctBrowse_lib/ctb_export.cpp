#include "ctb/ctb_export.h"


namespace ctb
{
   namespace
   {
      const auto joinValues = [](auto&& fields, char sep)
      {
         return fields | vws::join_with(sep) | rng::to<std::string>();
      };
   }   // namespace detail


   // clang-format off
   // because sometimes it does the stupidest, ugliest shit I swear

   [[nodiscard]] std::string exportData(const DatasetPtr& ds, std::span<std::pair<CtProp, std::string>> prop_mappings, char sep_char)
   {
      if (!ds or !ds->rowCount()) return "";

      if (rng::end(prop_mappings) != rng::find_if(prop_mappings, [&](const auto& mapping) { return !ds->hasProperty(mapping.first); }))
      {
         throw ctb::Error{ format("Dataset {} does not contain all of the requested export properties", ds->getCollectionName()) };
      }

      // write column headers first
      std::ostringstream strm{};
      strm << joinValues(prop_mappings | vws::transform([](const auto& mapping){ return mapping.second; }), sep_char) << '\n';

      auto row_pos = ds->rowPosition();
      ds->moveFirst();
      do
      {
         strm << joinValues(prop_mappings | vws::transform([&](const auto& mapping)
                                                            {
                                                               return ds->getProperty(mapping.first).asString();
                                                            }),
                                                            sep_char) << '\n';
      }
      while (ds->advanceRow(1));

      ds->moveToRow(row_pos);
      return strm.str();
   }


   [[nodiscard]] std::string exportData(const DatasetPtr& ds, std::span<CtProp> props, char sep_char)
   {
      using namespace detail;

      using PropNameMap = std::map<CtProp, std::string_view>;
      
      // This is a temp hack to replace some field names until I can whip up a UI to make it configurable.
      PropNameMap prop_mappings = {
         { CtProp::PendingOrderDate, "PurchaseDate"    },
         { CtProp::PendingStoreName, "PurchaseStore"   },
         { CtProp::Location,         "CurrentLocation" },
         { CtProp::WineAndSize,      "Wine"            }
      };

      auto getPropMapping = [&prop_mappings](const CtProp prop) -> std::pair<CtProp, std::string>
      {
         std::string prop_name{ prop_mappings[prop] };
         if (prop_name.empty()) prop_name = enum_to_string(prop);

         return std::make_pair(prop, prop_name);
      };

      auto props_to_export = props | vws::transform(getPropMapping) | rng::to<std::vector>();

      return exportData(ds, props_to_export, sep_char);
   }


}   // namespace ctb
