/*********************************************************************
 * @file       LabelImageCache.h
 *
 * @brief      declaration for the LabelImageCache class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "tasks.h"

#include <wx/image.h>

#include <expected>
#include <string>
#include <stop_token>
#include <vector>


namespace ctb::app
{

   /// @brief manages a disk-based cache of wine label images.
   ///
   /// note that while instances of this class are thread-safe,  you should be careful how 
   /// you use the return value of loadImage() if you're not in the main UI thread, since since
   /// using it with other UI code should only be done from the main UI thread.
   /// 
   class LabelImageCache final
   {
   public:
      /// @brief constructor
      /// @param cache_folder - path of folder to use for disk cache. env vars will be expanded
      /// 
      /// may throw if cache folder doesn't exist and can't be created, or is a relative path. 
      /// Any embedded environment variables contained in the folder path will be expanded.
      /// 
      explicit LabelImageCache(std::string cache_folder);
      ~LabelImageCache() noexcept;


      /// @brief result type for loadImage() method
      ///
      using LoadImageResult  = std::expected<wxImage, tasks::ResultCode>;


      /// @brief synchronously retrieve a wxImage for the requested label image
      /// @param task that will be waited on to retrieve the image file
      /// @return the requested wxBitmap if task was successful, a result code if not
      /// 
      /// the task will be waited on synchronously, if that is not desirable you can poll
      /// by calling task.is_ready() to decide when to call loadImage().
      /// 
      auto loadImage(tasks::FetchImageTask& task) -> LoadImageResult;


      /// @brief fetch a label image asynchronously.
      ///
      /// caller can check if result is ready by polling is_ready() on the returned task and 
      /// then calling loadImage() to retrieve the image when it's ready, or you can use
      /// one of the coro sync/when functions to synchronously wait for the image bytes and 
      /// load them directly into an object of your choosing.
      /// 
      auto fetchLabelImage(uint64_t wine_id) -> tasks::FetchImageTask;


      /// @brief launch a background task that will retrieve label images for the requested wines
      /// @return the task that should be polled or wait
      /// 
      auto updateCacheAsync(std::vector<uint64_t> wine_ids, std::stop_token cancel) -> tasks::UpdateCacheTask;
      

      /// @brief shuts down the thread pool, attempting to cancel any remaining tasks. 
      ///
      /// this function returns immediately, the shutdown is asynchronous
      /// 
      void shutdown() noexcept;

      // no default init or copy/assign
      LabelImageCache() = delete;
      LabelImageCache(const LabelImageCache&) = delete;
      LabelImageCache& operator=(const LabelImageCache&) = delete;

   private:
      const fs::path    m_cache_folder;   // const because modifying after construction wouldn't be thread-safe
      coro::thread_pool m_pool;
      std::stop_source  m_cancel_source{};

      static auto buildFileName(uint64_t wine_id, int image_num = 1) -> std::string
      {
         return ctb::format(constants::FMT_LABEL_IMAGE_FILENAME, wine_id, image_num);
      }

      static auto buildFilePath(fs::path folder, uint64_t wine_id, int image_num = 1) -> fs::path
      {
         return folder / buildFileName(wine_id, image_num);
      }
   };
}