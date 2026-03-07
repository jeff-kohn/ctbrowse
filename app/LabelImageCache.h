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
#include <wx/weakref.h>

#include <expected>
#include <memory>
#include <string>
#include <stop_token>
#include <vector>

namespace ctb::app
{
   // used to load pages requiring javascript
   class HiddenWebClient;


   /// @brief wxImageTask - adapts FetchFileTask to return wxImage
   ///
   /// This wrapper just adds a convenience method for returning the 
   /// future value as a wxImage instead of raw bytes. 
   class wxImageTask final : public tasks::FetchFileTask
   {
   public:
      using base          = tasks::FetchFileTask;
      using SharedFuture  = base::SharedFuture;
      using Future        = base::Future;
      using ResultWrapper = std::expected<wxImage, Error>;

      /// @brief wxImageTask constructor 
      explicit wxImageTask(SharedFuture f) noexcept : base{ std::move(f) }
      {}
      explicit wxImageTask(Future&& f) noexcept : base{ std::move(f) }
      {}

      /// @brief getImage() - retrieve the future value from the task as a wxImage
      /// 
      /// This is a potentially long, BLOCKING call if file is still being downloaded!
      ///
      /// @return expected - the requested wxImage; unexpected - ctb::Error describing the failure
      auto getImage() noexcept -> ResultWrapper;
   };



   /// @brief manages a disk-based cache of wine label images.
   ///
   /// The public interface of this class should only be called from the main UI thread, since it talks to 
   /// a wxWebView. (This class does use multi-threading internally though). 
   /// 
   class LabelImageCache final
   {
   public:
      /// @brief LabelImageCache constructor
      /// 
      /// @param cache_folder - path of folder to use for disk cache. env vars will be expanded
      /// @throws ctb::Error if cache folder doesn't exist and can't be created, or is a relative path. 
      explicit LabelImageCache(fs::path cache_folder, wxWeakRef<HiddenWebClient> web_client_ref = {});
      ~LabelImageCache() noexcept;

      /// @brief Fetch a label image asynchronously.
      ///
      /// Caller can check if result is ready by polling the returned task and 
      /// then calling getImage() to retrieve the image when it's ready
      /// 
      auto fetchLabelImage(uint64_t wine_id) -> std::expected<wxImageTask, ctb::Error>;

      /// @brief shuts down the thread pool, attempting to cancel any remaining tasks. 
      ///
      /// this function returns immediately, the shutdown is asynchronous. After calling shutdown,
      /// any calls to other methods on this instance will throw a ctb::Error.
      /// 
      void shutdown() noexcept;

      /// @brief Indicates whether a shutdown has been initiated. If it has any subsequent fetchLabelImage() calls will fail.
      /// @return true if shutdown has been initiated otherwise false.
      auto shutdownInitiated() const noexcept(false) -> bool
      {
         return m_cancel_source.stop_possible() == false;
      }

      // no default init or copy/assign
      LabelImageCache() = delete;
      LabelImageCache(const LabelImageCache&) = delete;
      LabelImageCache& operator=(const LabelImageCache&) = delete;

   private:
      class Request;
      using RequestPtr      = std::shared_ptr<Request>;
      using LabelRequestMap = std::unordered_map<uint64_t, RequestPtr>;

      LabelRequestMap              m_requests{};
      const fs::path               m_cache_folder;    // modifying after construction wouldn't be thread-safe anyways
      std::stop_source             m_cancel_source{}; // For signaling cancellation if we're shutting down.
      wxWeakRef<HiddenWebClient>   m_web_client_ref{};

      // "thread proc" for processing a label image request after we've download the wine-details html
      static void fetchLabelThreadProc(RequestPtr ptr, std::string page_text, fs::path folder, std::stop_token token);

      // Callback when a web page requested from the web client has been successfully loaded.
      void onPageLoaded(uint64_t wine_id, std::expected<std::string, ctb::Error> result);
   };
}