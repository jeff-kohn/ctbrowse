#include "HttpDownloader.h"
#include "IoManager.h"

#include "ctb/utility_http.h"

#include <glaze/net/http_client.hpp>
#include <memory>
#include <openssl/ssl.h>
#include <utility>

namespace ctb
{
   using namespace asio;


   /// @brief construct an HttpDownloader
   /// @param executor - if supplied, will be used to run HTTP requests. Otherwise a private
   ///                   executor running on a background thread will be used.
   HttpDownloader::HttpDownloader(any_io_executor executor)
      : m_ctx{ executor ? nullptr : std::make_unique<IoManager>() },
        m_client{ std::in_place, executor ? executor : m_ctx->get_executor() }
   {
      // configure http client to use TLS with Windows cert store for verification.
      m_client->configure_ssl_context(
         [](auto& ctx)
         {
            constexpr const char* STORE_NAME = "org.openssl.winstore://";
            if (!SSL_CTX_load_verify_store(ctx.native_handle(), STORE_NAME))
               SPDLOG_DEBUG("HttpDownloader failed to load Windows Cert Store for TLS.");
         });

      m_client->set_ssl_verify_mode(asio::ssl::verify_peer);
   }


   // needs to be here so impl members are complete types, only forward declared in header.
   HttpDownloader::~HttpDownloader()
   = default;


   /// @brief get an HTTP request asynchronously
   ///
   void HttpDownloader::getAsync(std::string_view url, const HttpHeaders& headers, HttpResultCallback callback)
   {
      m_client->get_async(url, headers,
                          [cbfunc = std::move(callback)](HttpResult result) mutable
                          {
                             if (result)   // NOLINT
                                SPDLOG_DEBUG("HttpDownloader HTTP request returned success. Status: {}", result->status_code);
                             else
                                SPDLOG_DEBUG("HttpDownloader HTTP request returned error - {}", result.error().message());

                             cbfunc(std::move(result));
                          });
   }


   void HttpDownloader::downloadTable(TableId table, const CredentialWrapper& cred, HttpResultCallback callback)
   {
      auto table_name  = enum_to_string(table);
      auto data_format = enum_to_string(DataFormatId::csv);

      auto url = ctb::format(
         constants::FMT_URL_CT_TABLE, percentEncode(cred.username()), percentEncode(cred.password()), data_format, table_name);

      getAsync(url, {}, std::move(callback));
   }


}   // namespace ctb
