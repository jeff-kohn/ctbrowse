#pragma once
#include "ctb/ctb.h"

#include "ctb/model/DatasetMgrAsyncCallbacks.h"
#include "ctb/tables/table_data.h"

#include "async/HttpDownloader.h"
#include "async/IoManager.h"
#include "browser/CellarTrackerBrowser.h"


namespace ctb
{

   /// @brief private implementation class for the async operations supported by CtDatasetMgr
   struct DatasetMgrAsyncImpl
   {
      static constexpr uint32_t NUM_CPU_THREADS = 2;
      static constexpr uint32_t NUM_IO_THREADS  = 1;

      // ordering is important here not only for initialization but also teardown
      fs::path                 label_folder{ ctb::format("{}/Labels", constants::CURRENT_DIRECTORY) };
      fs::path                 table_folder{ constants::CURRENT_DIRECTORY };
      IoManager                io_pool{};
      exec::static_thread_pool cpu_pool{ NUM_CPU_THREADS };
      CellarTrackerBrowser     browser{ io_pool.get_context() };
      HttpDownloader           http_client{ io_pool.get_executor() };

      /// @brief Get the file contents for the specified wine label.
      exec::task<ImageFileContents> sndGetLabelImage(uint64_t wine_id, stdexec::inplace_stop_token cancel_token) noexcept(false);

      /// @brief download a table file from CT website
      void downloadTableAsync(TableId                     table_id,
                              const CredentialWrapper&    cred,
                              TableResultCallback         notify_callback,
                              stdexec::inplace_stop_token cancel_token);

      /// @brief download a label image from CT website
      void retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback, stdexec::inplace_stop_token cancel_token);
   };

}   // namespace ctb
