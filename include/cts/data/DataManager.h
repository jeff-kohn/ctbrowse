#pragma once

#include "WineList.h"

#pragma warning(push)
#pragma warning(disable: 4365 4464 4702)
#include "cts/external/csv.hpp"
#pragma warning(pop)

#include <expected>
#include <filesystem>

namespace cts::data
{
   namespace fs = std::filesystem;
   
   class DataManager
   {
   public:
      DataManager(fs::path data_folder) : m_data_folder{ std::move(data_folder) }
      {}

      WineListData getWineList()
      {
         csv::CSVReader reader{ R"(C:\Users\jkohn\AppData\Roaming\cts_win\List.csv)" };
         WineListData wines{};
         WineListEntry entry{};
         for (csv::CSVRow& row : reader)
         {
            if (entry.parse(row))
               wines.emplace_back(std::move(entry));
         }
         return wines;
      }

   private:
      fs::path m_data_folder{};
      
   };

} // namespace cts::data
