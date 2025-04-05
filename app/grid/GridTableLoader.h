/*******************************************************************
 * @file GridTableLoader.h
 *
 * @brief Header file for the class GridTableLoader
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "interfaces/GridTableEvent.h"

#include <filesystem>
#include <memory>
#include <unordered_map>


namespace ctb::app
{
   namespace fs = std::filesystem;


   /// @brief class to manage a cached collection of grid tables, which are use for grid view in the application
   class GridTableLoader
   {
   public:
      /// @brief default ctor, initializes data folder to "." unless overriden by a call to setDataFolder()
      GridTableLoader() = default;

      /// @brief construct a GridTableLoader specifying the data folder. May throw if folder is invalid.
      ///
      explicit GridTableLoader(const fs::path& folder)
      {
         setDataFolder(folder);
      }

      /// @brief enum for the support grid tables
      ///
      enum class GridTableId
      {
         WineList,
         ReadyToDrinkList,
      };

      /// @brief specify the location for data files
      ///
      /// throws an exception if the folder doesn't exist
      void setDataFolder(const fs::path& folder)
      {
         if (not fs::exists(folder))
         {
            throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DataError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
         }
         m_data_folder = folder;
      }

      /// @brief returns the location used for loading data files from disk
      ///
      fs::path getDataFolder() const
      {
         return m_data_folder;
      }

      /// @brief the smart-ptr-to-base that this class returns to callers.
      ///
      using GridTablePtr = GridTablePtr;

      /// @brief get the requested grid table
      ///
      /// this will throw an exception if the table couldn't be loaded.
      ///
      GridTablePtr getGridTable(GridTableId tbl);

   private:
      fs::path m_data_folder{constants::CURRENT_DIRECTORY};
   };


} // namespace ctb::app