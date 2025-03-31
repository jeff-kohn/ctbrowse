/*********************************************************************
 * @file       LabelImageCache.cpp
 *
 * @brief      implementation for the LabelImageCache class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "LabelImageCache.h"
#include "tasks.h"

#include <ctb/utility.h>  
#include <coro/coro.hpp>
#include <wx/mstream.h>

#include <algorithm>
#include <fstream>


namespace ctb::app
{
   
   using namespace tasks;
   using std::unexpected;

   LabelImageCache::LabelImageCache(std::string cache_folder) :
      m_cache_folder{ expandEnvironmentVars(cache_folder) },
      m_pool{{
         .thread_count = 2,
         .on_thread_start_functor = [](auto id) { SPDLOG_DEBUG("thread pool worker {} is starting up.", id); },
         .on_thread_stop_functor = [](auto id) { SPDLOG_DEBUG("thread pool worker {} is shutting down.", id); }
      }}
   {
      if (m_cache_folder.is_relative() or !fs::is_directory(m_cache_folder))
         throw Error{ constants::ERROR_STR_RELATIVE_LABEL_CACHE };

      if (!fs::exists(m_cache_folder) )
      {
         if (!fs::create_directories(m_cache_folder))
            throw Error{ constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER };
      }
   }


   LabelImageCache::~LabelImageCache() noexcept
   {
      shutdown();
   }


   auto makeLabelDownloadTask(uint64_t wine_id, int image_num, std::stop_token token) -> tasks::FetchImageTask
   {
      SPDLOG_DEBUG("makeLabelDownloadTask({}, {}) now running.", wine_id, image_num);
      try 
      {
         // First, execute task to get the initial HTTP request for the wine page.
         checkStopToken(token);
         auto response = co_await makeHttpGetTask(getWineDetailsUrl(wine_id), token);
         validateOrThrow(response);

         SPDLOG_DEBUG("makeLabelDownloadTask({}, {}) successfully got wine details html.", wine_id, image_num);

         // parse the HTML to get the URL for the label image.
         checkStopToken(token);
         auto img_url = parseLabelUrlFromHtml(response->text);
         if (img_url.empty())
            throw Error{ constants::ERROR_STR_LABEL_URL_NOT_FOUND };

         SPDLOG_DEBUG("makeLabelDownloadTask({}, {}) parsed image url of {}.", wine_id, image_num, img_url);

         // now execute task to download the label image 
         checkStopToken(token);
         response = co_await makeHttpGetTask(img_url, token);
         validateOrThrow(response);


         // check if we got the response or need to retry

         // save it to disk.

         // return task that can be used to load the bytes into a wxImage

      }
      catch (std::exception& e)
      {
         log::exception(e);
         co_return unexpected{ ResultCode::Error };
      }
   }


   auto LabelImageCache::loadImage(tasks::FetchImageTask& task) -> LoadImageResult
   {
      try 
      {
         // retrieve the image data from the task, this may block if it's still executing
         auto bytes = coro::sync_wait(task);
         if (!bytes)
            return unexpected{ bytes.error() };

         // initialize a stream with the bytes returned from the task so we can load it into a wxImage
         wxMemoryInputStream byte_stream(bytes->data(), bytes->size());
         wxImage label_img{};
         label_img.LoadFile(byte_stream, wxBITMAP_TYPE_JPEG);
         return label_img;
      }
      catch (std::exception& e)
      {
         log::exception(e);
         return unexpected{ tasks::ResultCode::Error };
      }
   }


   void LabelImageCache::fetchLabelImage(uint64_t wine_id) 
   {
      constexpr auto image_num = 1; // might support more in the future.

      FetchImageTask task{};
      auto file_path = buildFilePath(m_cache_folder, wine_id, image_num);

      // Choose the appropriate task depending on if the file is found locally or needs to be downloaded.
      if (fs::exists(file_path))
      {
         m_pool.spawn(makeLoadFileTask(file_path, m_cancel_source.get_token()));
      }
      else
      {
         m_pool.spawn(makeLabelDownloadTask(wine_id, image_num, m_cancel_source.get_token()));
      }

      
   }

   //auto LabelImageCache::fetchLabelImage(uint64_t wine_id) -> tasks::FetchImageTask
   //{
   //   constexpr auto image_num = 1; // might support more in the future.

   //   FetchImageTask task{};
   //   auto file_path = buildFilePath(m_cache_folder, wine_id, image_num);

   //   // Choose the appropriate task depending on if the file is found locally or needs to be downloaded.
   //   if (fs::exists(file_path))
   //   {
   //      task = m_pool.schedule(makeLoadFileTask(file_path, m_cancel_source.get_token()));
   //   }
   //   else
   //   {
   //      task = m_pool.schedule(makeLabelDownloadTask(wine_id, image_num, m_cancel_source.get_token()));
   //   }

   //   // start the task, we don't want lazy eval
   //   m_pool.resume(task.handle()); 
   //   return task;
   //}


   auto LabelImageCache::updateCacheAsync(std::vector<uint64_t> wine_ids, std::stop_token cancel) -> tasks::UpdateCacheTask
   {
      return tasks::UpdateCacheTask{};
   }


   void LabelImageCache::shutdown() noexcept
   {
      // the thread_pool::shutdown() method will synchronously wait for any currently 
      // running (or queued) tasks, which could take a long time so we signal to 
      // cancel first, so the tasks know they shouldn't continue to run.
      m_cancel_source.request_stop();
      m_pool.shutdown();
   }




   /*


   std::optional<wxBitmap> LabelImageCache::getLabelImage(uint64_t wine_id)
   {
      coro::when_all
      // if we decide to support getting multiple images per wine, this could become a parameter.
      constexpr auto img_num = 1;

      auto file_path = buildFilePath(m_cache_folder, wine_id, img_num);
      
      wxBitmap img{};
      if (fs::exists(file_path) and img.LoadFile(file_path.generic_string().c_str(), wxBITMAP_TYPE_JPEG))
         return img;

      return {};
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
   */

} // namespace ctb::app
