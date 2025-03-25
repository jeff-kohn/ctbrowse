/*********************************************************************
 * @file       LabelImageCache.h
 *
 * @brief      declaration for the LabelImageCache class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"

#include <coro/task.hpp>
#include <coro/thread_pool.hpp>
#include <wx/bitmap.h>

#include <atomic>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>


namespace ctb::app
{

   /// @brief manages a disk-based cache of wine label images.
   ///
   /// this class is meant to be used from the main UI thread; and while you can use instances
   /// of it in other threads, instances are not threadsafe and should only be used from a single thread
   /// 
   /// note that the lifetime of the background cache thread is tied to the lifetime of the 
   /// LabelImageCache that started it, and destroying the LabelImageCache will cancel the background
   /// thread if it's running.
   /// 
   class LabelImageCache final
   {
   public:
      explicit LabelImageCache(std::string cache_folder);
      ~LabelImageCache() noexcept;


      /// @brief get a label image from the cache, if available
      /// @return the image if available, or std::nullopt if not
      /// 
      /// you can request that the image be fetched online if not available locally;
      /// but the online fetch happens in the background and this function will NOT
      /// block until it's available. If you get back an empty optional and 
      std::optional<wxBitmap> getLabelImage(uint64_t wine_id);


      /// @brief  fetches label images missing from the cache for the specified iWineId's
      /// @return true if a cache update was started, false if pendingCacheUpdate() == true
      ///         or an error occurred starting the background task.
      /// 
      /// queuing up requests is not supported (for now). this function returns 
      /// immediately, cache update is done in background thread.
      /// 
      bool requestCacheUpdate(const std::vector<uint64_t>& wine_ids);
      bool requestCacheUpdate(uint64_t wine_id)
      {
         return requestCacheUpdate(std::vector{ wine_id });
      }

      /// @brief convenience function that will take a CtDatset and transform it into a view of iWineId's and requesting an update on those id's
      ///
      template <rng::input_range Rng, CtRecordTraits traits> requires std::same_as<rng::range_value_t<Rng>, CtRecordImpl<traits> >
      bool requestCacheUpdate(Rng recs)
      {
         return requestCacheUpdate(recs | vws::transform([](auto rec) -> uint64_t { return rec[traits::iWineId]; }));
      }


      /// @brief check if a background cache update is currently running
      /// 
      bool pendingCacheUpdate() const
      {
         return m_thread_active.load();
      }


      /// @brief cancel any pending cache downloads
      ///
      /// if wait = true, this function will block until the cache thread has existed, otherwise
      /// it will just request the thread to stop and return immediately.
      /// 
      void cancelCacheUpdate(bool wait = false);


      // no default init or copy/assign
      LabelImageCache() = delete;
      LabelImageCache(const LabelImageCache&) = delete;
      LabelImageCache(LabelImageCache&&) = default;
      LabelImageCache& operator=(const LabelImageCache&) = delete;
      LabelImageCache& operator=(LabelImageCache&&) = default;

   private:
      std::optional<std::jthread> m_thread{};
      std::atomic<bool>           m_thread_active{ false };
      fs::path                    m_cache_folder{};

      static void workerThreadProc(std::stop_token cancel, fs::path cache_folder, std::vector<uint64_t> ids);

      enum class TaskResultCode
      {
         Success,
         Error,
         Aborted
      };
      using TaskResult = coro::task<TaskResultCode>;

      static TaskResult startDownloadTask(coro::thread_pool& tp, uint64_t wine_id);

      static std::string buildFileName(uint64_t wine_id, int image_num = 1)
      {
         return ctb::format(constants::FMT_LABEL_IMAGE_FILENAME, wine_id, image_num);
      }

      static fs::path buildFilePath(fs::path folder, uint64_t wine_id, int image_num = 1)
      {
         
         return folder / buildFileName(wine_id, image_num);
      }
   };
}