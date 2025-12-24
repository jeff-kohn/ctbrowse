/*********************************************************************
 * @file       LabelImageCache.h
 *
 * @brief      declaration for the LabelImageCache class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"

#include <ctb/tasks/tasks.h>
#include <wx/image.h>

#include <expected>
#include <memory>
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
      /// @brief LabelImageCache constructor
      /// 
      /// @param cache_folder - path of folder to use for disk cache. env vars will be expanded
      /// @throws ctb::Error if cache folder doesn't exist and can't be created, or is a relative path. 
      explicit LabelImageCache(std::string cache_folder);
      ~LabelImageCache() noexcept;


      /// @brief wxImageTask - adapts FetchFileTask to return wxImage
      ///
      /// This wrapper just adds a convenience method for returning the 
      /// future value as a wxImage instead of raw bytes. 
      class wxImageTask final : public tasks::FetchFileTask
      {
      public:
         using FetchFileTask = tasks::FetchFileTask;
         using FutureType    = FetchFileTask::FutureType;
         using ResultWrapper = std::expected<wxImage, Error>;

         /// @brief getImage() - retrieve the future value from the task as a wxImage
         /// 
         /// This is a potentially long, BLOCKING call if file is still being downloaded!
         ///
         /// @return expected - the requested wxImage; unexpected - ctb::Error describing the failure
         auto getImage() noexcept -> ResultWrapper;

         wxImageTask()                               = default;
         wxImageTask(wxImageTask&&)                  = default;
         ~wxImageTask() noexcept                     = default;
         wxImageTask& operator=(wxImageTask&&)       = default;
         wxImageTask(const wxImageTask&)             = delete;
         wxImageTask& operator=(const wxImageTask&)  = delete;

      private:
         /// @brief wxImageTask constructor (private, accessible only from LabelImageCache)
         explicit wxImageTask(FetchFileTask::FutureType&& t) noexcept : FetchFileTask{ std::move(t) }
         {}
         friend class LabelImageCache;
      };


      /// @brief Fetch a label image asynchronously.
      ///
      /// Caller can check if result is ready by polling the returned task and 
      /// then calling getImage() to retrieve the image when it's ready
      /// 
      auto fetchLabelImage(uint64_t wine_id) -> wxImageTask;

      /// @brief shuts down the thread pool, attempting to cancel any remaining tasks. 
      ///
      /// this function returns immediately, the shutdown is asynchronous. After calling shutdown,
      /// any calls to other methods on this instance will throw a ctb::Error.
      /// 
      void shutdown() noexcept;

      // no default init or copy/assign
      LabelImageCache() = delete;
      LabelImageCache(const LabelImageCache&) = delete;
      LabelImageCache& operator=(const LabelImageCache&) = delete;

   private:
      const fs::path    m_cache_folder;   // modifying after construction wouldn't be thread-safe anyways
      std::stop_source  m_cancel_source{};

      void checkShutdown() const noexcept(false)
      {
         if (!m_cancel_source.stop_possible())
            throw Error{ constants::ERROR_STR_LABEL_CACHE_SHUT_DOWN }; 
      }

      static auto buildLabelPath(const fs::path& folder, uint64_t wine_id) -> fs::path
      {
         return folder / buildLabelFilename(wine_id);
      }

      static auto buildLabelFilename(uint64_t wine_id) -> std::string
      {
         constexpr auto image_num = 1;
         return ctb::format(constants::FMT_LABEL_IMAGE_FILENAME, wine_id, image_num);
      }

      static auto runFetchAndSaveLabelTask(fs::path folder, uint64_t wine_id, std::stop_token token) noexcept(false) -> tasks::FetchFileTask::ReturnType;
   };

   using LabelCachePtr = std::shared_ptr<LabelImageCache>;
}