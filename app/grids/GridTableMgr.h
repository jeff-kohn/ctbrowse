/*******************************************************************
 * @file GridTableMgr.h
 *
 * @brief Header file for the class GridTableMgr
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "grids/GridTableBase.h"

#include <filesystem>
#include <memory>
#include <unordered_map>


namespace ctb::app
{
   namespace fs = std::filesystem;


   /// @brief class to manage a cached collection of grid tables, which are use for grid view in the application
   class GridTableMgr
   {
   public:
      /// @brief enum for the support grid tables
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
      fs::path getDataFolder() const
      {
         return m_data_folder;
      }


      /// @brief the smart-ptr-to-base that this class returns to callers.
      using GridTablePtr = GridTableBase::GridTablePtr;


      /// @brief get the requested grid table
      ///
      /// this will always return a valid object, but it may be empty (e.g.
      /// contain zero rows of data) if the table file couldn't be loaded
      GridTablePtr getGridTable(GridTableId tbl);


   private:
      using GridTables = std::unordered_map<GridTableId, GridTablePtr>;
      GridTables m_grid_tables{};
      fs::path m_data_folder{constants::CURRENT_DIRECTORY};

   };


} // namespace ctb::app