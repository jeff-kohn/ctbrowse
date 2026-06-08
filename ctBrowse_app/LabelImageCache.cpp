/*********************************************************************
 * @file       LabelImageCache.cpp
 *
 * @brief      implementation for the LabelImageCache class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "LabelImageCache.h"
#include "HiddenWebClient.h"

#include <ctb/tasks/tasks.h>
#include <ctb/utility.h>  
#include <wx/mstream.h>

#include <algorithm>
#include <fstream>
#include <thread>

namespace ctb::app
{

   using namespace ctb::tasks;
   using std::async;
   using std::launch;
   using std::unexpected;


   namespace
   {
      // helper functions used by thread proc, these throw on error

      inline auto buildLabelFilename(uint64_t wine_id) -> std::string
      {
         // we may want to support multiple images per wine in the future, but for now there will just be the one. 
         constexpr auto image_num = 1;
         return ctb::format(constants::FMT_LABEL_IMAGE_FILENAME, wine_id, image_num);
      }


      inline auto buildLabelPath(const fs::path& cache_folder, uint64_t wine_id) -> fs::path
      {
         return cache_folder / buildLabelFilename(wine_id);
      }


      inline auto getLabelImageUrl(const std::string& wine_details_html, std::stop_token& token) -> std::string
      {
         checkStopToken(token);

         auto img_url = parseLabelUrlFromHtml(wine_details_html);
         if (img_url.empty())
         {
            SPDLOG_DEBUG("Returned Wine Details HTML did not contain image  url:\n {}", wine_details_html);
            throw Error{ constants::ERROR_STR_LABEL_URL_NOT_FOUND };
         }

         log::info("getLabelImageUrl() parsed image url of '{}'", img_url);
         return img_url;
      }


      inline auto downloadImage(const std::string& img_url, std::stop_token& token) -> Buffer
      {
         checkStopToken(token);

         auto response = runHttpGetTask(img_url, token, getImageRequestHeaders());
         auto buf = viewResponseBytes(response);

         log::info("downloadImage() downloaded {} bytes.", buf.size());
         return Buffer{ std::from_range, buf };
      }


      inline auto saveImageFile(const fs::path& file_path, BufferSpan buf, std::stop_token& token)
      {
         checkStopToken(token);
         try
         {
            saveBinaryFile(file_path, buf, true);
            log::info("saveImageFile() successfully saved {} bytes to '{}'", buf.size(), file_path.generic_string());
         }
         catch (...)
         {
            log::error("Unabled to save downloaded label image ({} bytes) to {}. {}", file_path.generic_string(), buf.size(), packageError().formattedMessage());
         }
      }
   } // namespace
   
   
   auto wxImageTask::getImage() noexcept -> ResultWrapper
   {
      try
      {
         // this is a potentially long, BLOCKING call if file is still being downloaded!
         auto bytes = getValue();
         if (!bytes)
            return unexpected{ std::move(bytes.error()) };

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


   /// @brief Request - class representing a request to retrieve a wine's label image from the web 
   class LabelImageCache::Request
   {
   public:
      explicit Request(uint64_t wine_id) :
         m_task{ m_promise.get_future() },
         m_wine_id{ wine_id }
      {}

      auto wineId() const -> uint64_t 
      {
         return m_wine_id;
      }

      auto task() const -> const wxImageTask& 
      {
         return m_task;
      }

      void setValue(wxImageTask::ReturnType&& value) 
      {
         m_promise.set_value(std::move(value));
      }

      void setError(const std::exception_ptr& ep = std::current_exception())
      {
         m_promise.set_exception(ep);
      }

   private:
      using Promise = std::promise<wxImageTask::ReturnType>;

      Promise     m_promise{};
      wxImageTask m_task;       // will be intialized with m_promise's future, and returned to caller via task()
      uint64_t    m_wine_id{};
   };


   LabelImageCache::LabelImageCache(fs::path cache_folder, const wxWeakRef<HiddenWebClient>& web_client_ref) :  
      m_cache_folder{ std::move(cache_folder) },
      m_web_client_ref{ web_client_ref }

   {
      if (m_cache_folder.is_relative() or (fs::exists(m_cache_folder) and !fs::is_directory(m_cache_folder)))
      {
         throw Error{ constants::ERROR_STR_INVALID_LABEL_CACHE };
      }

      if (!fs::exists(m_cache_folder) )
      {
         // MS in their infinite wisdom, will return false even though the directory was created if the string had a trailing slash.
         // So we have to ignore return value and check for an error_code
         std::error_code ms_sucks{};
         fs::create_directories(m_cache_folder, ms_sucks);
         if (ms_sucks)
         {
            throw Error{ constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER };
         }
      }
   }


   auto LabelImageCache::fetchLabelImage(uint64_t wine_id) -> std::expected<wxImageTask, ctb::Error>
   {
      if (shutdownInitiated())
         throw Error{ constants::ERROR_STR_LABEL_CACHE_SHUT_DOWN };

      // Choose the appropriate task depending on if the file is found locally or needs to be downloaded.
      auto file_path = buildLabelPath(m_cache_folder, wine_id);
      if (fs::exists(file_path))
      {
         // we don't need to save it to our map for later wxWebView callback, just return the request directly since
         // it will load the file direcly from disk.
         return wxImageTask{ async(launch::deferred, runLoadFileTask, file_path, m_cancel_source.get_token()) };
      }
      else if (m_web_client_ref == nullptr)
      {
         return std::unexpected{ Error{ Error::Category::NotSupported, "Online label fetching disabled, backend webclient not available." } };
      }

      // do we already have a pending request we can return?
      if (auto it = m_requests.find(wine_id); it != m_requests.end())
      {
         return it->second->task();
      }

      // create and save request object, so that we can get it later for second step of image retrieval. 
      auto req = std::make_unique<Request>(wine_id);
      auto [request_iter, was_inserted] = m_requests.try_emplace(wine_id, std::move(req));
      assert(was_inserted);

      // We have to request the initial page from webClient so we can then parse it to get the image URL.
      auto url = getWineDetailsUrl(wine_id);
      if (m_web_client_ref->requestPage(url, [this, wine_id](auto&& callback) { onPageLoaded(wine_id, callback); }))
      {
         return wxImageTask{ request_iter->second->task() };
      }
      else {
         // couldn't submit the request, so remove it from the map and return an error.
         m_requests.erase(request_iter);
         return unexpected{ Error{ Error::Category::GenericError, constants::FMT_ERROR_STR_WEB_CLIENT_REQUEST_REJECTED, url} };
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


   // NOLINTNEXTLINE(performance-unnecessary-value-param) 
   void LabelImageCache::fetchLabelThreadProc(RequestPtr request, std::string page_text, fs::path cache_folder, std::stop_token token) // cppcheck-suppress passedByValueCallback
   {
      try
      {
         // parse the HTML to get the URL for the label image.
         auto url    = getLabelImageUrl(page_text, token);
         auto buffer = downloadImage(url, token);
         auto path   = buildLabelPath(cache_folder, request->wineId());

         saveImageFile(path, buffer, token); 
         request->setValue(std::move(buffer));
      }
      catch (...)
      {
         auto err = packageError();
         if (err.category == Error::Category::OperationCanceled)
         {
            log::info("LabelImageCache::fetchLabelThreadProc({}) terminating early due to cancellation/shutdown", request->wineId());
         }
         else {
            log::warn("LabelImageCache::fetchLabelThreadProc({}) terminating with exception: {}", request->wineId(), err.formattedMessage());
         }
         request->setError(std::make_exception_ptr(err));
      }
   }


   void LabelImageCache::onPageLoaded(uint64_t wine_id, std::expected<std::string, ctb::Error> result)
   {
      if (shutdownInitiated())
      {
         log::warn("LabelImageCache::onPageLoaded aborting due to initiated shutdown.");
         return;
      }

      auto request_iter = m_requests.find(wine_id);
      if (request_iter == m_requests.end())
      {
         log::error("LabelImageCache::onPageLoaded called but no matching request was found for wine_id {}", wine_id);
      }
      else
      {
         if (result.has_value())
         {
            log::info("Received web page requested for wine_id {}", wine_id);
            std::thread{ fetchLabelThreadProc, std::move(request_iter->second), std::move(*result), m_cache_folder, m_cancel_source.get_token() }.detach();
            m_requests.erase(request_iter); 
         }
      }
   }


} // namespace ctb::app
