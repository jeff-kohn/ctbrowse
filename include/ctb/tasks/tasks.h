/*********************************************************************
 * @file       tasks.h
 *
 * @brief      contains some functions (aka tasks) that can be executed
 *             synchronously or asynchronously, with cancellation support
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/utility_http.h"
#include "ctb/tasks/PollingTask.h"

#include <cpr/api.h>
#include <cpr/cprtypes.h>

#include <stop_token>
#include <string>


namespace ctb::tasks
{
   namespace fs = std::filesystem;


   /// @brief helper function, throws exception if stop_token.stop_requested() == true
   /// 
   /// @throws ctb::Error if stop_token if stop has been request
   /// 
   inline auto checkStopToken(const std::stop_token& token) noexcept(false) -> void
   {
      if (token.stop_requested())
         throw Error{ constants::ERROR_STR_OPERATION_CANCELED, Error::Category::OperationCanceled };
   }

   
   /// @brief Task type use for LoadFile, SaveFile, LabelDownload tasks while all return file bytes.
   ///
   using FetchFileTask = PollingTask<Buffer>;

   /// @brief creates a task to load a binary file from disk into a buffer.
   /// @param file - path of the file to read
   /// @param token - cancellation support
   /// @return  the requested file bytes
   /// @throws ctb::Error if the operation fails
   /// 
   auto runLoadFileTask(fs::path file, std::stop_token token = {}) noexcept(false) -> FetchFileTask::ReturnType;
   
   
   /// @brief  result type and task type for HTTP Request task
   ///
   using HttpRequestResult = cpr::Response;
   using HttpRequestTask   = PollingTask<HttpRequestResult>;

   /// @brief Run a HTTP GET request for the specified URL
   /// @param url - url for the request
   /// @param token - cancellation support
   /// @return the HTTP response returned by the request
   /// @throws ctb::Error if the operation fails
   /// 
   //auto runHttpGetTask(std::string url, std::stop_token token = {}) noexcept(false) -> HttpRequestTask::ReturnType;

   /// @brief Run a HTTP GET request for the specified URL
   /// @param url - url for the request
   /// @param token - cancellation support
   /// @param args - variadic args to pass to the cpr request
   /// @return the HTTP response returned by the request
   /// @throws ctb::Error if the operation fails
   /// 
   template<typename... CprArgs>
   auto runHttpGetTask(std::string url, std::stop_token token, CprArgs...args) noexcept(false) -> HttpRequestTask::ReturnType
   {
      checkStopToken(token);
      return validateOrThrow(cpr::Get(cpr::Url{ url }, args...));
   }


   /// @brief Runs a task to download a label image from CT website.
   /// 
   /// This task downloads the wine details page, parses it to find the img url, and 
   /// then downloads the image and loads the image bytes into a buffer.
   /// 
   /// @param wine_id - id of the wine to download image for
   /// @param token - cancellation support
   /// @return a buffer containing the requested image's bytes
   /// @throws ctb::Error if file couldn't be downloaded.
   /// 
   auto runLabelDownloadTask(uint64_t wine_id, std::stop_token token = {}) noexcept(false) -> FetchFileTask::ReturnType;

} // namespace ctb::tasks
