#pragma once

#include "WineList.h"


#include <excpected>
#include <filesystem>

namespace cts::data
{
   namespace fs = std::filesystem;
   
   class DataManager
   {
   public:
      DataManager(fs::path data_folder) : m_data_folder{ std::move(data_folder) };


   private:
      fs::path m_data_folder{};
   };

} // namespace cts::data
