#pragma once
#include "ctb/ctb.h"
#include "ctb/model/DatasetMgrAsyncCallbacks.h"
#include "ctb/tables/table_data.h"

#include "browser/CellarTrackerBrowser.h"

#include "async/HttpDownloader.h"
#include "async/IoManager.h"
#include "async/senders.h"
#include <exec/async_scope.hpp>

namespace ctb
{

   /// @brief private implementation class for the async operations supported by CtDatasetMgr
   class DatasetMgrAsyncImpl
   {
   public:
      static constexpr uint32_t NUM_CPU_THREADS = 2;
      static constexpr uint32_t NUM_IO_THREADS  = 1;


      /// @brief Specify the location for table files.
      DatasetMgrAsyncImpl& setTableFolder(const std::string& folder);


      /// @brief returns the location used for loading data files from disk
      const std::string& getTableFolder() const;


      /// @brief specify the location for label images. Environment variables will be expanded
      ///
      /// @throws ctb::Error if the folder doesn't exist and can't be created.
      DatasetMgrAsyncImpl& setLabelImageFolder(const std::string& folder);


      /// @brief returns the location used for loading label images from disk
      const std::string& getLabelImageFolder() const;


      /// @brief download a table file from CT website
      void downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableResultCallback notify_callback);


      /// @brief download a label image from CT website
      void retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback);


      /// @brief Navigates to CT.com and logs in with the supplied credentials if browser isn't already logged into CT
      /// @param cred - creds to use
      void checkBrowserLoginAsync(CredentialWrapper cred, LoginResultCallback result_callback);


      /// @brief Sender that can be used to safely notify an async caller about an error during the operation
      ///        This sender will signal the callback with the error information, ensuring that the callback
      ///        happens on a CPU pool thread and that no uncaught exceptions escape.
      template<typename CallbackT>
      auto safeErrorCallback(CallbackT&& callback, std::exception_ptr ep) noexcept
      {
         return ctb::senders::safeErrorCallback(m_cpu_pool.get_scheduler(), std::forward<CallbackT>(callback), move(ep));
      }


      /// @brief start the headless browser for async image downloads
      void startBrowser(std::string browser_path, std::string data_dir, int32_t port);


      /// @brief Initiates shutdown of async IO services in the background.
      /// @return true if shutdown commencing, false if something went wrong.
      bool requestShutdown();


      /// @brief indicates whether shutdown is in progress (or already completed)
      bool shutdownRequested() const;


      /// @brief syncrhonous method to wait for shutdown to complete.Will return immediately
      ///        if shutdown is already completed or not initiated
      /// @return - true if shutdown is complete, false if shutdown was not yet initiated
      bool waitForShutdown();

      DatasetMgrAsyncImpl() = default;
      ~DatasetMgrAsyncImpl() noexcept;
      DatasetMgrAsyncImpl(DatasetMgrAsyncImpl&)                  = delete;
      DatasetMgrAsyncImpl(DatasetMgrAsyncImpl&&)                 = delete;
      DatasetMgrAsyncImpl& operator=(const DatasetMgrAsyncImpl&) = delete;
      DatasetMgrAsyncImpl& operator=(DatasetMgrAsyncImpl&&)      = delete;

   private:
      // ordering is important here not only for initialization but also teardown
      IoManager                m_io_pool{};
      exec::static_thread_pool m_cpu_pool{ NUM_CPU_THREADS };
      exec::async_scope        m_job_scope{};
      CellarTrackerBrowser     m_browser{ m_io_pool.get_context() };
      HttpDownloader           m_http_client{ m_io_pool.get_executor() };
      std::string              m_label_folder{ ctb::format("{}/Labels", constants::CURRENT_DIRECTORY) };
      std::string              m_table_folder{ constants::CURRENT_DIRECTORY };

      /// @brief Get the file contents for the specified wine label.
      exec::task<ImageFileContents> sndGetLabelImage(uint64_t wine_id) noexcept(false);

      void throwIfShuttingDown() noexcept(false);
   };

}   // namespace ctb

