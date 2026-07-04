#include "HeadlessWebClient.h"
#include "ctb/tasks/async_tasks.h"
#include "ctb/utility_templates.h"


#include <fmt/chrono.h>
#include <glaze/glaze_exceptions.hpp>
#include <string_view>


namespace ctb::web
{
   constexpr auto MAX_CONNECT_RETRIES = 10;
   constexpr auto FMT_EDGE_HTTP_URL   = "http://127.0.0.1:{}/json/version";
   constexpr auto FMT_EDGE_ARGS       = "--headless=new --remote-debugging-address=127.0.0.1 "
                                        "--no-first-run --no-default-browser-check --disable-sync "
                                        "--remote-debugging-port={} --disable-gpu "
                                        "--user-data-dir=\"{}\""sv;

   struct BrowserInfoMsg
   {
      std::string Browser;
   };

   HeadlessWebClient::HeadlessWebClient(ContextPtr io_ctx) : m_ctx{ io_ctx }, m_ws_client{ io_ctx }
   {
      setupHandlers();
   }


   void HeadlessWebClient::start(std::string browser_path, std::string data_dir, int32_t port) noexcept(false)
   {
      tryExpandEnvironmentVars(browser_path);
      tryExpandEnvironmentVars(data_dir);
      if (m_client_status != Status::Stopped)
      {
         throw ctb::Error{ Error::Category::GeneralError, "HeadlessWebClient status is '{}', start() is not a valid operation.",
                           enum_to_string(m_client_status.load()) };
      }
      if (!fs::exists(browser_path))
      {
         throw ctb::Error{ Error::Category::FileError, "Couldn't launch headless web client, path \"{}\" does not exist.", browser_path };
      }
      if (!fs::exists(data_dir))
      {
         if (!createFolderPath(data_dir))
         {
            throw Error{ Error::Category::GeneralError,
                         "Couldn't launch headless web client, data dir \"{}\" does not exist and could not be created.", data_dir };
         }
      }

      // First we need to launch the browser process.
      m_browser_handles = getValueOrThrow(win32::createProcessJob(browser_path, ctb::format(FMT_EDGE_ARGS, port, data_dir)));
      m_client_status.store(Status::Starting);

      // Establish the websocket connection
      auto url = ctb::format(FMT_EDGE_HTTP_URL, port);
      attemptWebsocketConnect(url, MAX_CONNECT_RETRIES);
   }

   void HeadlessWebClient::attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay)
   {
      // We need to use an HTTP GET to retrieve the WS endpoint and connect. This callback will run on the io_context's
      // thread.
      auto callback = [this, url, retries, retry_delay](HttpDownloader::HttpResult result) mutable
      {
         try
         {
            auto response = tasks::validateHttpResponse(result).response_body;
            SPDLOG_DEBUG("Got HTTP GET response from browser: {}", response);
            StringMap props{};
            glz::ex::read_json(props, response);

            auto ws_url = props["webSocketDebuggerUrl"];
            m_ws_client.connect(ws_url);

            /// We have the HTTP response, not parse WS connect URL from it and connect our websocket_client...
         }
         catch (...)
         {
            auto error = packageError();
            if (retries == 0)
            {
               m_client_status.store(Status::Stopped);
               m_browser_handles = {};
               SPDLOG_DEBUG("HeadlessWebClient couldn't establish connection with browser. {}", error.formattedMessage());
            }
            else
            {
               SPDLOG_DEBUG("Browser not ready, retrying in {}... ({} retries left)", retry_delay, retries);
               runWithDelay(retry_delay,
                            [this, url = std::move(url), retries, retry_delay]() mutable
                            {
                               attemptWebsocketConnect(std::move(url), retries - 1U, retry_delay * 2);
                            });
            }
         }
      };

      m_http_client.getAsync(url, {}, std::move(callback));
   }


   void HeadlessWebClient::setupHandlers()
   {
      m_ws_client.on_open(std::bind_front(&HeadlessWebClient::onOpen, this));
      m_ws_client.on_close(std::bind_front(&HeadlessWebClient::onClose, this));
      m_ws_client.on_message(std::bind_front(&HeadlessWebClient::onMessage, this));
      m_ws_client.on_error(std::bind_front(&HeadlessWebClient::onError, this));
   }


   void HeadlessWebClient::onOpen()
   {
      SPDLOG_DEBUG("HeadlessWebClient::onOpen - websocket connection established.");
      m_client_status.store(Status::Ready);
   }


   void HeadlessWebClient::onClose(glz::ws_close_code code, std::string_view reason)
   {
      SPDLOG_DEBUG("HeadlessWebClient::onClose - code: {}, reason: '{}'", static_cast<uint16_t>(code), reason);
      m_client_status.store(Status::Stopped);
      m_browser_handles = {};   // will kill the process.
   }


   void HeadlessWebClient::onMessage(std::string_view message, glz::ws_opcode opcode)
   {

      if (opcode == glz::ws_opcode::text)
      {
         BrowserMessage msg{};
         auto ec = glz::read_json(msg, message);
      }
      else
      {
         SPDLOG_DEBUG("Unexpected opcode received in HeadlessClient::onMessage. Opcode: {}. Message: {}", enum_to_string(opcode), message);
      }
   }


   void HeadlessWebClient::onError(std::error_code ec)
   {
      SPDLOG_DEBUG("HeadlessWebClient::onError - {} ({})", ec.message(), ec.value());
   }


   void HeadlessWebClient::runWithDelay(std::chrono::milliseconds delay, std::move_only_function<void()> func)
   {
      asio::co_spawn(
         *m_ctx,
         [this, func = std::move(func), delay]() mutable -> asio::awaitable<void>
         {
            // The timer is a local variable inside the coroutine!
            asio::steady_timer timer(*m_ctx, delay);

            // Suspend this mini-coroutine without blocking the IO thread
            co_await timer.async_wait(asio::use_awaitable);
            func();
         },
         asio::detached);
   }

}   // namespace ctb::web


