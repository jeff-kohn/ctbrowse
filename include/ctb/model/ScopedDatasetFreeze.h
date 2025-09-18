
#pragma once

#include "ctb/ctb.h"
#include "ctb/interfaces/IDataset.h"

namespace ctb
{
   /// @brief Small RAII wrapper to support freezing/unfreezing a dataset.
   class ScopedDatasetFreeze final
   {
   public:
      explicit ScopedDatasetFreeze(DatasetPtr dataset) : m_dataset{ std::move(dataset) }
      {
         freeze();
      }

      ~ScopedDatasetFreeze() noexcept
      {
         try 
         {
            unfreeze();
         }
         catch (...) {
            SPDLOG_DEBUG("ScopedDatasetFreeze destructor caught unexpected exception. {}", packageError().formattedMesage());
         }
      }

      // manually freeze the dataset
      void freeze()
      {
         if (m_dataset)
         {
            m_dataset->freezeData();
         }
      }

      // manually unfreeze the dataset
      void unfreeze()
      {
         if (m_dataset)
         {
            m_dataset->unfreezeData();
         }
      }

      // there's really no good reason to copy or move these objects
      ScopedDatasetFreeze() = delete;
      ScopedDatasetFreeze(const ScopedDatasetFreeze&) = delete;
      ScopedDatasetFreeze(ScopedDatasetFreeze&&) = delete;
      ScopedDatasetFreeze& operator=(const ScopedDatasetFreeze&) = delete;
      ScopedDatasetFreeze& operator=(ScopedDatasetFreeze&&) = delete;

   private:
      DatasetPtr m_dataset{};
   };
}