/*********************************************************************
 * @file       LabelImageCache.cpp
 *
 * @brief      implementation for the LabelImageCache class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "LabelImageCache.h"

#include <coro/coro.hpp>
#include <ctb/winapi_util.h>
#include <algorithm>

namespace ctb::app
{
   using PoolOptions = coro::thread_pool::options;
   
   PoolOptions pool_opts
      {
         .thread_count = std::thread::hardware_concurrency(),
         .on_thread_start_functor = [](auto id) { SPDLOG_DEBUG("thread pool worker {} is starting up.", id); },
         .on_thread_stop_functor = [](auto id) { SPDLOG_DEBUG("thread pool worker {} is shutting down.", id); }
      };


   LabelImageCache::LabelImageCache(std::string cache_folder)
   {
      util::tryExpandEnvironmentVars(cache_folder);

      m_cache_folder = cache_folder;
      if (m_cache_folder.is_relative())
         throw Error{ constants::ERROR_STR_RELATIVE_LABEL_CACHE };

      if (!fs::exists(m_cache_folder) or !fs::is_directory(cache_folder))
      {
         if (!fs::create_directories(m_cache_folder))
            throw Error{ constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER };
      }
   }


   LabelImageCache::~LabelImageCache() noexcept
   {
      try 
      {
         cancelCacheUpdate(true);
      }
      catch(std::exception& e)
      {
         log::exception(e);
      }
   }


   std::optional<wxBitmap> LabelImageCache::getLabelImage(uint64_t wine_id)
   {
      // if we decide to support getting multiple images per wine, this could become a parameter.
      constexpr auto img_num = 1;

      auto file_path = buildFilePath(m_cache_folder, wine_id, img_num);
      
      wxBitmap img{};
      if (fs::exists(file_path) and img.LoadFile(file_path.generic_string().c_str(), wxBITMAP_TYPE_JPEG))
         return img;

      return {};
   }


   bool LabelImageCache::requestCacheUpdate(const std::vector<uint64_t>& wine_ids)
   {
      if (m_thread_active)
         return false;

      // launch background thread to do the work and return immediately.
      m_thread_active.store(true);
      m_thread = std::jthread{ &LabelImageCache::workerThreadProc, m_cache_folder, wine_ids };
      return true;
   }


   void LabelImageCache::cancelCacheUpdate(bool wait)
   {
      if (m_thread)
      {
         if (m_thread_active)
         {
            log::info("Requesting cancelation of label image cache thread.");
            m_thread->request_stop();
            if (wait and m_thread->joinable())
            {
               log::info("Waiting for label image cache thread to terminate...");
               m_thread->join();
               log::info("Label image cache thread terminated");
            }
         }
         else
         {
            // thread has already completed so discard it.
            m_thread = std::nullopt;
         }
      }
   }


   auto LabelImageCache::startDownloadTask(coro::thread_pool& tp, uint64_t wine_id) -> TaskResult
   {
      // move execution from main worker thread to thread pool.
      co_await tp.schedule();

      SPDLOG_DEBUG("Returning success from label download task for id {}", wine_id);
      co_return TaskResultCode::Success;
   }


   void LabelImageCache::workerThreadProc(std::stop_token cancel, fs::path cache_folder, std::vector<uint64_t> ids)
   {
      if (cancel.stop_requested())
         return;

      // first determine the files we need to request.
      log::info("Label cache queue contains {} id's, scanning for missing files...", ids.size());
      std::erase_if(ids, [cache_folder, &cancel](auto id) 
         {  
            auto fp = buildFilePath(cache_folder, id);
            return fs::exists(fp); 
         });
      log::info("{} id's missing from label cache.", ids.size());

      // now run the request task for each needed file, using a thread pool.
      coro::thread_pool pool{ pool_opts };

      auto download_tasks = vws::all(ids) | vws::transform([&pool](auto id) -> TaskResult { return startDownloadTask(pool, id); })
                                          | rng::to<std::vector>();

      auto results = coro::sync_wait(coro::when_all(std::move(download_tasks)));
      auto success_count = std::count_if(results.begin(), results.end(), [](auto& result) -> bool 
         {  
            return result.return_value() == TaskResultCode::Success; 
         });

      log::info("Label image cache update processed {} tasks successfully and failed {} tasks.", success_count, results.size() - success_count);

   }


} // namespace ctb
