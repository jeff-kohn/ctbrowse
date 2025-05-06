/*******************************************************************
 * @file DatasetLoader.h
 *
 * @brief Header file for the class DatasetLoader
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/table_data.h"
#include "ctb/model/DatasetBase.h"

#include <filesystem>
#include <memory>
#include <unordered_map>


namespace ctb::app
{
   namespace fs = std::filesystem;


   /// @brief class to load dataset files from disk.
   ///
   class DatasetLoader
   {
   public:
      /// @brief default ctor, initializes data folder to "." unless overriden by a call to setDataFolder()
      DatasetLoader() = default;

      /// @brief construct a DatasetLoader specifying the data folder. May throw if folder is invalid.
      ///
      explicit DatasetLoader(const fs::path& folder) noexcept(false)
      {
         setDataFolder(folder);
      }

      /// @brief specify the location for data files
      ///
      /// throws an exception if the folder doesn't exist
      void setDataFolder(const fs::path& folder) noexcept(false)
      {
         if (not fs::exists(folder))
         {
            throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DataError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
         }
         m_data_folder = folder;
      }

      /// @brief returns the location used for loading data files from disk
      ///
      auto getDataFolder() const -> fs::path
      {
         return m_data_folder;
      }

      /// @brief Get the requested dataset
      ///
      /// this will throw an exception if the table couldn't be loaded.
      ///
      auto getDataset(TableId tbl) -> DatasetPtr;

   private:
      fs::path m_data_folder{constants::CURRENT_DIRECTORY};
   };


} // namespace ctb::app