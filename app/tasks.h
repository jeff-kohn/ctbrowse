#pragma once

#include <ctb/utility_http.h>
#include <cpr/response.h>
#include <coro/task.hpp>

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


   /// @brief creates a task to load a binary file from disk into a buffer.
   ///
   /// task will be suspended until waited on or resumed() with a thread_pool
   /// 
   auto makeLoadFileTask(fs::path file, std::stop_token token = {}) -> FetchImageTask;


   // result type and task type for UpdateCache task
   using UpdateCacheResult = std::vector<ResultCode>;
   using UpdateCacheTask   = coro::task<UpdateCacheResult>;

   /// @brief creates a task to update the label image cache, downloading any missing images
   /// 
   /// task will be suspended until waited on or resumed() with a thread_pool
   /// 
   auto makeUpdateCacheTask() -> UpdateCacheTask;


   // result type and task type for HTTP Request task
   using HttpRequestResult = std::expected<cpr::Response, ctb::Error>;
   using HttpRequestTask   = coro::task<HttpRequestResult>;

   /// @brief creates a task to execute an HTTP request using CPR
   ///
   /// task will be suspended until waited on or resumed() with a thread_pool
   /// 
   auto makeHttpGetTask(std::string_view url, std::stop_token token = {}) -> HttpRequestTask;


   /// @brief validates the supplied task result, and throws if validation fails
   /// @return always returns true unless it throws (useful in conditionals)
   /// @throws ctb::Error
   /// 
   inline auto validateOrThrow(const HttpRequestResult& task_result) -> bool
   {
      // could be the HTTP request itself failed...
      if (!task_result)
         throw task_result.error();

      // or it could be that the response indicates an error...
      auto result = validateResponse(*task_result);
      if (!result)
         throw result.error();

      return true;
   }


   /// @brief helper function, throws exception if stop_token.stop_requested() == true
   /// @throws ctb::Error
   /// 
   inline void checkStopToken(const std::stop_token& token)
   {
      if (token.stop_requested())
         throw Error{ constants::ERROR_STR_OPERATION_CANCELED, Error::Category::OperationCanceled };
   }


} // namespace ctb::tasks
