#include "ctb/ctb.h"
#include "ctb/interfaces/IDataset.h"
#include "ctb/tables/CtSchema.h"
#include <sstream>

namespace ctb
{

   namespace detail
   {
      auto joinValues = [](auto&& fields, char sep)
      {
         return fields | vws::join_with(sep) | rng::to<std::string>();
      };
   }   // namespace detail


   // clang-format off
   // because sometimes it does the stupidest, ugliest shit I swear


   template<rng::input_range PropsT> requires std::same_as<std::remove_cvref_t<rng::range_value_t<PropsT>>, CtProp>
   [[nodiscard]] std::string exportData(DatasetPtr ds, const PropsT& props, char sep_char = '\t')
   {
      using namespace detail;

      if (!ds or !ds->rowCount()) return "";

      if (rng::end(props) != rng::find_if(props, [&](CtProp p) { return !ds->hasProperty(p); }))
      {
         throw ctb::Error{ format("Dataset {} does not contain all of the requested export properties", ds->getCollectionName()) };
      }

      // This is a temp hack to replace some field names until I can whip up a UI to make it configurable.
      std::map<CtProp, std::string_view> prop_mappings = {
         { CtProp::PendingOrderDate, "PurchaseDate" },
         { CtProp::PendingStoreName, "PurchaseStore"},
         { CtProp::Location, "CurrentLocation"}
      };

      // write column headers first
      std::ostringstream strm{};
      strm << joinValues(props | vws::transform([&prop_mappings](CtProp p)
         {
            auto it = prop_mappings.find(p);
            return it == prop_mappings.end() ? enum_to_string(p) : it->second;
         }), sep_char) << '\n';

      auto row_pos = ds->rowPosition();
      ds->moveFirst();
      do
      {
         strm << joinValues(props | vws::transform([&](CtProp p) { return ds->getProperty(p).asString(); }), sep_char) << '\n';
      }
      while (ds->advanceRow(1));

      ds->moveToRow(row_pos);
      return strm.str();
   }

   template<rng::input_range PropsT> requires std::same_as<std::remove_cvref_t<rng::range_value_t<PropsT>>, CtProp>
   [[nodiscard]] std::string exportData(DatasetPtr ds, std::span<std::pair<CtProp, std::string_view>> prop_mappings, char sep_char = '\t')
   {
      using namespace detail;

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
                                                               auto& prop =  ds->getProperty(mapping.first);
                                                               return prop.hasString() ? prop.asStringView() : prop.asString();
                                                            }),
                                                            sep_char) << '\n';
      }
      while (ds->advanceRow(1));

      ds->moveToRow(row_pos);
      return strm.str();
   }

}  // namespace ctb
