#pragma once
#include "ctb/ctb.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/model/CtDatasetOptions.h"
#include "ctb/model/ProReviewsCache.h"


namespace ctb::app
{
   struct TableDownloadNotification
   {
      TableId table_id{};
      size_t  file_size{};

   };
   /// @brief High-level class that simplifies downloading tables, loading them from disk into dataset, downloading
   ///        label images, getting cache tables, etc.Uses async I/O for downloading files and saving them to disk.
   ///
   class CtDatasetManager
   {
   public:
      // Used for downloading table files, expected value is the contents of the file
      using TableResult         = std::expected<std::string, ctb::Error>;
      using TableResultCallback = copyable_function<void(TableResult) const>;

      // used for loading (or downloading) images, expected value is the image bytes.
      using ImageResult         = std::expected<Buffer, ctb::Error>;
      using ImageResultCallback = copyable_function<void(ImageResult) const>;

      ~CtDatasetManager() noexcept;   //= default;

      /// @brief construct a CtDatasetLoader specifying the data folder. May throw if folder is invalid and can't be created.
      explicit CtDatasetManager(const fs::path& table_folder) noexcept(false);


      /// @brief specify the location for table files
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetManager&;


      /// @brief returns the location used for loading data files from disk
      auto getTableFolder() const -> const fs::path&;


      /// @brief specify the location for label images
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setLabelImageFolder(const fs::path& folder) noexcept(false) -> CtDatasetManager&;


      /// @brief returns the location used for loading label images from disk
      auto getLabelImageFolder() const -> const fs::path&;


      /// @brief Load a dataset with default options
      auto loadDataset(TableId table_id) -> DatasetPtr;


      /// @brief Load a dataset and apply options
      auto loadDataset(const CtDatasetOptions& options) -> DatasetPtr;


      /// @brief Retrieves the pro reviews cache.
      auto getProReviewsCache() -> ProReviewsCache&;


      /// @brief Download the specified table from CellarTracker.com in the background and save it to the data
      ///        folder, overwriting existing file.
      void downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableResultCallback notify_callback);


      /// @brief Download multiple tables from CellarTracker.com in the background and save it to the data folder,
      ///        overwriting existing files.
      template<rng::input_range RngT> requires std::same_as<rng::range_value_t<RngT>, TableId>
      void downloadTablesAsync(const RngT& tables, const CredentialWrapper& cred, TableResultCallback result_callback)
      {
         for (auto tbl_id : tables)
         {
            downloadTableAsync(tbl_id, cred, result_callback);
         }
      }


      void retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback);


      CtDatasetManager();
      CtDatasetManager(CtDatasetManager&&)                 = default;
      CtDatasetManager& operator=(CtDatasetManager&&)      = delete;
      CtDatasetManager(const CtDatasetManager&)            = delete;
      CtDatasetManager& operator=(const CtDatasetManager&) = delete;

   private:
      struct AsyncImpl;

      std::optional<ProReviewsCache> m_pro_cache{};
      fs::path                       m_table_folder{ constants::CURRENT_DIRECTORY };
      fs::path                       m_label_folder{ ctb::format("{}/Labels", constants::CURRENT_DIRECTORY) };
      indirect<AsyncImpl>            m_impl;
   };

}   // namespace ctb::app
