#pragma once

#include "ctb/ctb.h"   // NOLINT
#include "ctb/table_download.h"
#include "ctb/utility_http.h"

#include <asio/io_context.hpp>
#include <glaze/net/http_client.hpp>


#include <thread>
#include <utility>

namespace ctb
{

   class TableDownloader
   {
   public:
      using HttpResult = std::expected<glz::response, std::error_code>;

      TableDownloader()
      {
         // configure http client to use TLS with Windows cert store for verification.
         m_client.configure_ssl_context(
            [](auto& ctx)
            {
               constexpr const char* STORE_NAME = "org.openssl.winstore://";
               if (SSL_CTX_load_verify_store(ctx.native_handle(), STORE_NAME)) // NOLINT
                  SPDLOG_DEBUG("TableDownloader successfully loaded Windows Cert Store for TLS.");
               else
                  SPDLOG_DEBUG("TableDownloader failed to load Windows Cert Store for TLS.");
            });

         m_client.set_ssl_verify_mode(asio::ssl::verify_peer);

         // start the background thread for our async HTTP requests
         m_net_thread = std::jthread(
            [this]()
            {
               m_http_ctx.run();
            });
      }

      ~TableDownloader() noexcept
      {
         try
         {
            m_http_ctx.stop();
         }
         catch (...) // NOLINT
         {}   
      }

      template<typename Callback>
      void downloadTable(const CredentialWrapper& cred, TableId table, Callback callback)

      {
         auto table_name  = enum_to_string(table);
         auto data_format = enum_to_string(DataFormatId::csv);

         auto url = ctb::format(constants::FMT_URL_CT_TABLE,
                                percentEncode(cred.username()),
                                percentEncode(cred.password()),
                                data_format,
                                table_name);

         m_client.get_async(url, {},
                            [cbfunc = std::move(callback)](HttpResult result) mutable
                            {
                               if (result) // NOLINT
                                  SPDLOG_DEBUG("TableDownloader HTTP request returned success. Status: {}", result->status_code);
                               else
                                  SPDLOG_DEBUG("TableDownloader HTTP request returned error - {}", result.error().message());

                               cbfunc(std::move(result));
                            });
      }

      TableDownloader(TableDownloader&&)                = delete;
      TableDownloader(const TableDownloader&)           = delete;
      TableDownloader& operator=(TableDownloader&&)     = delete;
      TableDownloader& operator=(const TableDownloader) = delete;

   private:
      asio::io_context m_http_ctx{};
      glz::http_client m_client{ m_http_ctx.get_executor() };
      std::jthread     m_net_thread;

      using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;
      WorkGuard m_work_guard{ asio::make_work_guard(m_http_ctx) };
   };

}   // namespace ctb
