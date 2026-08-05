#pragma once
#include "ctb/ctb.h"
#include "ctb/model/DatasetMgrAsyncCallbacks.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/model/CtDatasetOptions.h"
#include "ctb/model/ProReviewsCache.h"

#include <stdexec/stop_token.hpp>


namespace ctb
{
   using MaybeStopToken = std::optional<stdexec::inplace_stop_token>;

   // options struct for dataset mgr. the path strings will have any embedded environment variables expanded before use
   struct DatasetMgrOptions
   {
      static constexpr int32_t           DEFAULT_BROWSER_WS_PORT   = 9222;
      static constexpr const char* const DEFAULT_BROWSER_DATA_PATH = R"(%LOCALAPPDATA%\ctBrowse for Windows\WebView)";
      static constexpr const char* const DEFAULT_BROWSER_EXE_PATH  = R"(%ProgramFiles(x86)%\Microsoft\Edge\Application\msedge.exe)";

      /// @brief Folder where table files downloaded from CT will be read/written from/to. ENV
      ///        vars will be expanded. Passing an invalid path will throw an exception.
      std::string table_folder{};

      /// @brief Folder where image label files will be stored when downloaded. ENV vars will
      ///        be expanded. Passing an invalid path will throw an exception.
      std::string label_folder{};

      /// @brief Path of the browser to use for headless page loads to download images.
      ///        ENV vars will be expanded. To disable image download, set to an empty
      ///        string. Passing an invalid path will throw an exception.
      std::string browser_path{ DEFAULT_BROWSER_EXE_PATH };

      /// @brief Path of the data folder to use for headless page loads to download images.
      ///        ENV vars will be expanded. Ignored if browser_path.empty(). If path doesn't
      ///        exist and cannot be created, an exception will be thrown
      std::string browser_data_dir{ DEFAULT_BROWSER_DATA_PATH };

      /// @brief port # to use for local websocket communications with headless browser. Ignored if
      ///        browser_path.empty()
      int32_t browser_ws_port{ DEFAULT_BROWSER_WS_PORT };
   };


   // for PIMPL
   class DatasetMgrAsyncImpl;


   /// @brief High-level class that simplifies downloading tables, loading them from disk into dataset, downloading
   ///        label images, getting cache tables, etc. Uses async I/O for downloading files and saving them to disk.
   ///
   /// Note:  To get image download functionality you must construct instances of this class with a DatasetMgrOptions
   ///        containing a valid browser_path. The headless browser used for downloading images will not be started
   ///        if you default-construct an instance of this class or if you pass an empty string for browser_path.
   class CtDatasetMgr
   {
   public:
      ~CtDatasetMgr() noexcept;   //= default;

      /// @brief construct a CtDatasetLoader using the specified options.
      CtDatasetMgr(const DatasetMgrOptions& opts) noexcept(false);


      /// @brief post-construction initialization
      void init(const DatasetMgrOptions& opts);


      /// @brief Specify the location for table files. Environment variables will be expanded
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setTableFolder(const std::string& folder) noexcept(false) -> CtDatasetMgr&;


      /// @brief Specify the location for table files. Environment variables will NOT be expanded
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetMgr&;


      /// @brief returns the location used for loading data files from disk
      auto getTableFolder() const -> const std::string&;


      /// @brief specify the location for label images. Environment variables will be expanded
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setLabelImageFolder(const std::string& folder) noexcept(false) -> CtDatasetMgr&;


      /// @brief specify the location for label images. Environment variables will NOT be expanded
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      auto setLabelImageFolder(const fs::path& folder) noexcept(false) -> CtDatasetMgr&;


      /// @brief returns the location used for loading label images from disk
      auto getLabelImageFolder() const -> const std::string&;


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


      /// @brief Asynchronous function to retrieve a label image, either from local disk cache or HTTP download
      /// @param wine_id - the wine to get a label for
      /// @param result_callback - callback to receive the result when the async operation completes
      void retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback);


      /// @brief Navigates to CT.com and logs in with the supplied credentials if browser isn't already logged into CT
      /// @param cred - creds to use
      void checkBrowserLoginAsync(CredentialWrapper cred, LoginResultCallback result_callback);


      /// @brief Begins shutdown of async IO manager in the background. Can be called during app exit to make teardown more efficient
      bool requestShutdown();

      /// @return - true if shutdown is been requested, false otherwise. 
      bool shutdownRequested() const;

      // default ctor doesn't start the CellarTrackerBrowser service, would need to call init() later.
      CtDatasetMgr();
      CtDatasetMgr(CtDatasetMgr&&)                 = default;
      CtDatasetMgr& operator=(CtDatasetMgr&&)      = delete;
      CtDatasetMgr(const CtDatasetMgr&)            = delete;
      CtDatasetMgr& operator=(const CtDatasetMgr&) = delete;

   private:
      indirect<DatasetMgrAsyncImpl>  m_impl;
      std::optional<ProReviewsCache> m_pro_cache{};
   };

}   // namespace ctb
