/*********************************************************************
 * @file       LabelImageCache.cpp
 *
 * @brief      implementation for the LabelImageCache class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "LabelImageCache.h"

#include <ctb/task/tasks.h>
#include <ctb/utility.h>  
#include <wx/mstream.h>

#include <algorithm>
#include <fstream>


namespace ctb::app
{
   using namespace ctb::tasks;
   using std::async;
   using std::launch;
   using std::unexpected;


   auto LabelImageCache::wxImageTask::getImage() noexcept -> ResultWrapper
   {
      try
      {
         // this is a potentially long, BLOCKING call if file is still being downloaded!
         auto bytes = getValue();
         if (!bytes)
            return unexpected{ bytes.error() };

         // initialize a stream with the bytes returned from the task so we can load it into a wxImage
         wxMemoryInputStream byte_stream(bytes->data(), bytes->size());
         wxImage label_img{};
         label_img.LoadFile(byte_stream, wxBITMAP_TYPE_JPEG);
         return label_img;
      }
      catch (...) {
         return unexpected{ packageError() };
      }

   }


   LabelImageCache::LabelImageCache(std::string cache_folder) : m_cache_folder{ expandEnvironmentVars(cache_folder) }
   {
      if (m_cache_folder.is_relative() or ( (fs::exists(m_cache_folder) and !fs::is_directory(m_cache_folder)) ))
      {
         throw Error{ constants::ERROR_STR_RELATIVE_LABEL_CACHE };
      }

      if (!fs::exists(m_cache_folder) )
      {
         // MS in their infinite wisdom, will return false even though the directory was created if the string had a trailing slash.
         // This is FUCKING STUPID, and we have to work around it. 
         std::error_code ms_sucks{};
         fs::create_directories(m_cache_folder, ms_sucks);
         if (ms_sucks)
         {
            throw Error{ constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER };
         }
      }
   }


   auto LabelImageCache::fetchLabelImage(uint64_t wine_id) -> wxImageTask
   {
      checkShutdown();

      // Choose the appropriate task depending on if the file is found locally or needs to be downloaded.
      auto file_path = buildLabelPath(m_cache_folder, wine_id);
      if (fs::exists(file_path))
      {
         return wxImageTask{ async(launch::deferred, runLoadFileTask, file_path, m_cancel_source.get_token()) };
      }
      else {
         return wxImageTask{ async(launch::async, runFetchAndSaveLabelTask, m_cache_folder, wine_id, m_cancel_source.get_token()) };
      }
   }


   LabelImageCache::~LabelImageCache() noexcept
   {
      shutdown();
   }


   void LabelImageCache::shutdown() noexcept
   {
      // note that since we're currently just using std::async() we can't really join any of the the background tasks
      // we launched, only the caller holding the future can do that. The best we can do is signal cancellation and
      // stop accepting new tasks. If we move to a real async/concurrency library we should be able to do better.
      if (m_cancel_source.stop_possible()) m_cancel_source.request_stop();

      // now set invalid source, so we won't be able to launch more tasks.
      m_cancel_source = std::stop_source{ std::nostopstate };
   }


   auto LabelImageCache::runFetchAndSaveLabelTask(fs::path folder, uint64_t wine_id, std::stop_token token) noexcept(false) -> FetchFileTask::ReturnType
   {
      try
      {
         SPDLOG_DEBUG("runFetchAndSaveLabelTask({}, {}) starting execution", folder.generic_string(), wine_id);
         checkStopToken(token);
         auto buffer = runLabelDownloadTask(wine_id, token);

         checkStopToken(token);
         saveBinaryFile(buildLabelPath(folder, wine_id), buffer, true);

         return buffer;
      }
      catch (...)
      {
         auto err = packageError();
         if (err.category == Error::Category::OperationCanceled)
         {
            log::info("runFetchAndSaveLabelTask({}) terminating early due to cancellation/shutdown", wine_id);
         }
         else {
            log::warn("runFetchAndSaveLabelTask({}) terminating with exception: {}", wine_id, err.formattedMesage());
         }
         throw err;
      }

   }

} // namespace ctb::app
