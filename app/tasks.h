#pragma once

#include <ctb/utility.h>
#include <cpr/response.h>
#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <coro/thread_pool.hpp>

#include <expected>
#include <stop_token>
#include <string>
#include <vector>


namespace ctb::tasks
{
   namespace fs = std::filesystem;


   /// @brief result code used for our async tasks to indicate final status
   /// 
   enum class ResultCode
   {
      Success,
      PartialSuccess,
      Error,
      Aborted
   };


   /// @brief Result type and task type for FetchImage task. FileLoadTask also uses this result type.
   ///
   using ImageBytes        = std::vector<char>;
   using FetchImageResult  = std::expected<ImageBytes, ResultCode>;
   using FetchImageTask    = coro::task<FetchImageResult>;

   /// @brief creates a task to load a binary file from disk into a vector.
   ///
   /// task will be scheduled for eager execution on the specified thread_pool, function will 
   /// return immediately.
   /// 
   auto makeFileLoadTask(fs::path file, coro::thread_pool& tp, std::stop_token token = {}) -> FetchImageTask;


   // result type and task type for UpdateCache task
   using UpdateCacheResult = std::vector<ResultCode>;
   using UpdateCacheTask   = coro::task<UpdateCacheResult>;

   /// @brief creates a task to update the label image cache, downloading any missing images
   /// 
   /// task will be scheduled for eager execution on the specified thread_pool, function will 
   /// return immediately.
   /// 
   auto makeUpdateCacheTask() -> UpdateCacheTask;


   // result type and task type for HTTP Request task
   using HttpRequestResult = std::expected<cpr::Response, ResultCode>;
   using HttpRequestTask   = coro::task<HttpRequestResult>;

   /// @brief creates a task to execute an HTTP request using CPR
   ///
   /// task will be scheduled for eager execution on the specified thread_pool, function will 
   /// return immediately.
   /// 
   auto makeHttpGetTask(std::string_view url, coro::thread_pool& tp, std::stop_token token = {}) -> HttpRequestTask;


} // namespace ctb::tasks
