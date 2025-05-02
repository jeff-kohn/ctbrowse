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
#include "ctb/CredentialWrapper.h"
#include "ctb/utility_http.h"
#include "ctb/task/PollingTask.h"

#include <cpr/response.h>

#include <stop_token>
#include <string>
#include <vector>


namespace ctb::tasks
{
   namespace fs = std::filesystem;

   /// @brief Task type use for LoadFile, SaveFile, LabelDownload tasks while all return file bytes.
   ///
   using FetchFileTask = PollingTask<Buffer>;

   /// @brief creates a task to load a binary file from disk into a buffer.
   /// 
   auto runLoadFileTask(fs::path file, std::stop_token token = {}) noexcept(false) -> FetchFileTask::ReturnType;

   
   /// @brief  result type and task type for HTTP Request task
   using HttpRequestResult = cpr::Response;
   using HttpRequestTask   = PollingTask<HttpRequestResult>;

   /// @brief runs a task to execute an HTTP request
   ///
   auto runHttpGetTask(std::string url, std::stop_token token = {}) noexcept(false) -> HttpRequestTask::ReturnType;


   /// @brief Runs a task to download a label image from CT website.
   /// 
   /// This task downloads the wine details page, parses it to find the img url, and 
   /// then downloads the image and loads the image bytes into a buffer.
   /// 
   /// @return a buffer containing the requested image's bytes
   /// @throws ctb::Error if file couldn't be downloaded.
   /// 
   auto runLabelDownloadTask(uint64_t wine_id, std::stop_token token) noexcept(false) -> FetchFileTask::ReturnType;


   using LoginResult = cpr::Cookies;
   using LoginTask   = PollingTask<LoginResult>;

   /// @brief Runs a task to create a logon session for interacting with the CellarTracker website
   /// 
   /// Connects to the CT website using the supplied credential and retrieves user Cookies
   /// for connecting to and interacting with CT website.
   /// 
   /// @return the requested cookies if successful, a ctb::Error if unsuccessful.
   /// @throws ctb::Error if 
   /// 
   auto runCellarTrackerLogin(CredentialWrapper&& cred, std::stop_token token) noexcept(false) -> LoginTask::ReturnType;


   /// @brief helper function, throws exception if stop_token.stop_requested() == true
   /// 
   /// @throws ctb::Error if stop_token if stop has been request
   /// 
   inline auto checkStopToken(const std::stop_token& token) noexcept(false) -> void
   {
      if (token.stop_requested())
         throw Error{ constants::ERROR_STR_OPERATION_CANCELED, Error::Category::OperationCanceled };
   }




} // namespace ctb::tasks
