/*********************************************************************
* @file       CtDatasetExport.h
*
* @brief      Declaration for the class ExportOptions
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#pragma once

#include "App.h"

#include <ctb/ctb.h>
#include <ctb/interfaces/IDataset.h>
#include <ctb/tables/CtSchema.h>
#include <ctb/tables/table_data.h>

#include <string>

namespace ctb::app
{

   struct CtDatasetExportOptions
   {
      static constexpr std::string_view DEFAULT_EXTENSION      = "txt";
      static constexpr std::string_view DEFAULT_EXTENSION_NAME = "Tab Separated Values";

      // which table the export options are for.
      TableId table_id{ TableId::List };

      // this is the identifier for the saved export config, and is also used as the default export filename.
      std::string collection_name{ getTableDescription(table_id) };

      // just a safe global default that prevents an empty column list
      std::set<CtProp> props_to_export{ CtProp::WineAndVintage };

      // default is just a file name, no path.
      fs::path target_path{ ctb::format("{}.{}", collection_name, DEFAULT_EXTENSION) };
   };


   /// @brief get export options for a dataset
   ///
   /// collection name is the "primary key". if no saved options are found for the collection a default instance will be returned.
   auto getDefaultExportOptions(const DatasetPtr &ds) -> CtDatasetExportOptions;


   /// @brief
   /// @param collection_name
   /// @param table_id
   /// @return
   auto getOptionsForCollection(std::string_view collection_name, TableId table_id) -> std::expected<CtDatasetExportOptions, ctb::Error>;


   /// @brief serialize the options to the config store
   /// @param options
   void saveDefaultExportOptions(const CtDatasetExportOptions &export_options) noexcept(false);


}   // namespace ctb::app
