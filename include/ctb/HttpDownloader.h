#pragma once
#include "ctb/ctb.h"

#include "ctb/CredentialWrapper.h"
#include "ctb/tables/table_data.h"

#include <asio/any_io_executor.hpp>
#include <glaze/net/http_router.hpp>

#include <expected>
#include <memory>
#include <string_view>


namespace glz
{
   class http_client;
}

namespace ctb
{

   class BackgroundThreadContext;

   /// @brief Asynchronously downloads files (table data, image labels) from CT website
   ///
   /// This class can run on a caller-provided asio executor, or if one is not provided
   /// a private executor running on a background thread will be used. Instances of this 
   /// object must outlive the HTTP requests they execute, because they are stateful 
   /// even when using an externally-provided executor.
   /// 
   class HttpDownloader
   {
   public:
      using HttpHeaders        = std::unordered_map<std::string, std::string>;
      using HttpResult         = std::expected<glz::response, std::error_code>;
      using HttpResultCallback = std::move_only_function<void(HttpResult)>;


      /// @brief construct an HttpDownloader
      /// @param executor - if supplied, will be used to run HTTP requests. Otherwise a private
      ///                   executor running on a background thread will be used.
      HttpDownloader(asio::any_io_executor executor = asio::any_io_executor());

      /// @brief get an HTTP request asynchronously
      /// @param url - the url to send the GET request to
      /// @param headers - headers to send with the request
      /// @param callback -
      void getAsync(std::string_view url, const HttpHeaders& headers, HttpResultCallback callback);

      /// @brief Download a Table CSV file from CellarTracker.com via async HTTP
      /// @param table - the table to download
      /// @param cred  - credentials to use
      /// @param callback - callback to receive the result when it's ready. Will be called from background
      ///                   asio io_context thread.
      void downloadTable(TableId table, const CredentialWrapper& cred, HttpResultCallback callback);



      ~HttpDownloader();
      HttpDownloader(HttpDownloader&&)                = default;
      HttpDownloader& operator=(HttpDownloader&&)     = delete;
      HttpDownloader(const HttpDownloader&)           = delete;
      HttpDownloader& operator=(const HttpDownloader) = delete;

   private:
      // We may or may not run our own io_context in background thread depending on whether
      // this object was initialized with an external executor or not.
      using ContextPtr = std::unique_ptr<BackgroundThreadContext>;
      ContextPtr m_ctx{};

      // needs to be declared after m_ctx because of init order.
      indirect<glz::http_client> m_client;
   };

}   // namespace ctb
