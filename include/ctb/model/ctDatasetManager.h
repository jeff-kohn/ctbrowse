#pragma once
#include "ctb/ctb.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/model/CtDatasetOptions.h"
#include "ctb/model/ProReviewsCache.h"


namespace ctb::app
{

   /// @brief High-level class that simplifies downloading tables, loading them from disk into dataset, downloading
   ///        label images, getting cache tables, etc.Uses async I/O for downloading files and saving them to disk.
   ///
   class CtDatasetManager
   {
   public:
      /// @brief callback type used  for table download requests. Needs to be copyable because downloadTablesAsync
      ///        needs to copy the callable for each downloadTableAsync() call. The expeced return value is the
      ///        FQ path of the file that was downloaded.
      using TableDownloadResultCallback = copyable_function<void(std::expected<std::string, ctb::Error>)>;

      /// @brief default ctor, initializes data folder to "." unless overridden by a call to setTableFolder()
      CtDatasetManager();             // = default;
      ~CtDatasetManager() noexcept;   //= default;

      /// @brief construct a CtDatasetLoader specifying the data folder. May throw if folder is invalid and can't be created.
      explicit CtDatasetManager(const fs::path& table_folder) noexcept(false);


      /// @brief specify the location for data files
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetManager&;


      /// @brief returns the location used for loading data files from disk
      auto getTableFolder() const -> const fs::path&;


      /// @brief Load a dataset with default options
      auto loadDataset(TableId table_id) -> DatasetPtr;


      /// @brief Load a dataset and apply options
      auto loadDataset(const CtDatasetOptions& options) -> DatasetPtr;


      /// @brief Retrieves the pro reviews cache.
      auto getProReviewsCache() -> std::optional<ProReviewsCache>;


      /// @brief Download the specified table from CellarTracker.com in the background and save it to the data
      ///        folder, overwriting existing file.
      void downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableDownloadResultCallback notify_callback);


      /// @brief Download multiple tables from CellarTracker.com in the background and save it to the data folder,
      ///        overwriting existing files.
      template<rng::input_range RngT>   //requires std::same_as<std::remove_cvref<rng::range_value_t<RngT>>, TableId>
      void downloadTablesAsync(const RngT& tables, const CredentialWrapper& cred, TableDownloadResultCallback notify_callback)
      {
         for (auto tbl_id : tables)
         {
            downloadTableAsync(tbl_id, cred, notify_callback);
         }
      }


      CtDatasetManager(CtDatasetManager&&)                 = default;
      CtDatasetManager& operator=(CtDatasetManager&&)      = delete;
      CtDatasetManager(const CtDatasetManager&)            = delete;
      CtDatasetManager& operator=(const CtDatasetManager&) = delete;

   private:
      struct AsyncImpl;

      fs::path            m_data_folder{ constants::CURRENT_DIRECTORY };
      indirect<AsyncImpl> m_impl;
   };

}   // namespace ctb::app
