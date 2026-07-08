#include "HeadlessWebClient.h"
#include "ctb/tasks/async_tasks.h"
#include "ctb/utility_templates.h"


#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include <glaze/glaze_exceptions.hpp>
#include <string_view>


namespace ctb::webclient
{
   constexpr auto MAX_CONNECT_RETRIES = 10;
   constexpr auto FMT_EDGE_HTTP_URL   = "http://127.0.0.1:{}/json/version";
   constexpr auto FMT_EDGE_ARGS       = "--headless=new --remote-debugging-address=127.0.0.1 "
                                        "--remote-debugging-port={} --disable-gpu "
                                        "--no-first-run --no-default-browser-check --disable-sync "
                                        "--user-data-dir=\"{}\""sv;


   HeadlessWebClient::TargetSession::~TargetSession() noexcept
   {
      try
      {
         // this object may not be valid if it was moved-from
         if (m_web_client)
         {
            m_web_client->postCloseSession(std::move(m_session_id));
         }
      }
      catch (...)
      {
         SPDLOG_DEBUG("HeadlessWebClient::TargetSession::~TargetSession caught exception closing session. {}",
                      packageError().formattedMessage());
      }
   }


   HeadlessWebClient::TargetSession::TargetSession(TargetSession&& other) noexcept
      : m_session_id{ std::move(other.m_session_id) },
        m_web_client{ other.m_web_client }
   {
      other.m_web_client = nullptr;
   }


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


   void HeadlessWebClient::stop()
   {
      m_client_status.store(Status::ShuttingDown);
      postCommand(commands::CLOSE_BROWSER, {}, {});
      m_ws_client.close();
   }


   HeadlessWebClient::Status HeadlessWebClient::status() const
   {
      return m_client_status.load();
   }


   asio::awaitable<HeadlessWebClient::TargetSession> HeadlessWebClient::coroCreateSession() noexcept(false)
   {
      // make sure we're on the io thread.
      co_await asio::dispatch(*m_ctx, asio::use_awaitable);

      auto CheckResult = [](const BrowserMessage& response, std::string_view cmd_name)
      {
         if (response.error.has_value())
         {
            throw ctb::Error{ Error::Category::NetworkError, "{} command {} received error result - {}", cmd_name, response.id.value_or(-1),
                              response.error->str };
         }
         if (!response.result.has_value())
         {
            throw ctb::Error{ Error::Category::ParseError, "{} command {} received empty result.", cmd_name, response.id.value_or(-1) };
         }
      };

      // clang-format off

      auto* cmd_str = commands::CREATE_TARGET;
      auto response = co_await coroSendCommand(cmd_str, JsonPropMap{ { params::TARGET_URL, params::ABOUT_BLANK } }, {});
      CheckResult(response, cmd_str);

      auto target_result = glz::ex::read_json<CreateTargetResult>(response.result.value().str);
      SPDLOG_DEBUG("HeadlessWebClient::coroCreateSession - {} command received targetId '{}'", cmd_str, target_result.targetId);

      // now attach to the target to get a session
      cmd_str = commands::ATTACH_TARGET;
      JsonPropMap params{ { params::TARGET_ID, target_result.targetId }, { params::FLATTEN, true } };
      response = co_await coroSendCommand(cmd_str, std::move(params), {});
      CheckResult(response, cmd_str);

      auto session_result = glz::ex::read_json<AttachTargetResult>(response.result.value().str);
      co_return TargetSession{ *this, std::move(session_result.sessionId) };
   }


   void HeadlessWebClient::postCloseSession(std::string session_id) noexcept
   {
      postCommand(commands::CLOSE_TARGET, { { params::TARGET_ID, std::move(session_id) } }, {});
   }


   // clang-format on


   [[nodiscard]] asio::awaitable<BrowserMessage> HeadlessWebClient::coroSendCommand(std::string command,
                                                                                    JsonPropMap parameters,
                                                                                    MaybeString session_id) noexcept(false)
   {
      return asio::async_initiate<decltype(asio::use_awaitable), void(BrowserMessage)>(
         [this](auto handler, std::string command, JsonPropMap parameters, MaybeString session_id)
         {
            BrowserCommand msg{ .method    = std::move(command),
                                .id        = m_next_id++,
                                .sessionId = std::move(session_id),
                                .params    = std::move(parameters) };

            // map the completion handler to id so that we can look it up and complete it when the browser message comes back
            m_pending_requests[msg.id.transform(to_unsigned).value_or(0U)] = std::move(handler);

            // serialize and send the message. response will come via on_message()
            auto json = glz::ex::write_json(msg);
            m_ws_client.send(json);
         },
         asio::use_awaitable, std::move(command), std::move(parameters), std::move(session_id));
   }


   void HeadlessWebClient::postCommand(std::string command, JsonPropMap parameters, MaybeString session_id) noexcept
   {
      try
      {
         asio::co_spawn(
            *m_ctx,
            [this, cmd = std::move(command), params = std::move(parameters), id = std::move(session_id)] -> asio::awaitable<void>
            {
               try
               {
                  (void)co_await coroSendCommand(std::move(cmd), std::move(params), std::move(id));
               }
               catch (...)
               {
                  SPDLOG_DEBUG("HeadlessWebClient::postCommand failed: {}", packageError().formattedMessage());
               }
            },
            asio::detached);
      }
      catch (...)   // NOLINT
      {
         SPDLOG_DEBUG("HeadlessWebClient::postCommand failed: {}", packageError().formattedMessage());
      }
   }


   void HeadlessWebClient::setupHandlers()
   {
      m_ws_client.on_open(std::bind_front(&HeadlessWebClient::onWebSocketOpen, this));
      m_ws_client.on_close(std::bind_front(&HeadlessWebClient::onWebSocketClose, this));
      m_ws_client.on_message(std::bind_front(&HeadlessWebClient::onWebSocketMessage, this));
      m_ws_client.on_error(std::bind_front(&HeadlessWebClient::onWebSocketError, this));
   }


   void HeadlessWebClient::onWebSocketOpen()
   {
      SPDLOG_DEBUG("HeadlessWebClient::onOpen - websocket connection established.");
      m_client_status.store(Status::Ready);
   }


   void HeadlessWebClient::onWebSocketClose(glz::ws_close_code code, std::string_view reason)
   {
      SPDLOG_DEBUG("HeadlessWebClient::onClose - code: {}, reason: '{}'", static_cast<uint16_t>(code), reason);
      m_client_status.store(Status::Stopped);
      m_browser_handles = {};   // kill the browser process.
   }


   void HeadlessWebClient::onWebSocketMessage(std::string_view msg_text, glz::ws_opcode opcode)
   {
      if (opcode == glz::ws_opcode::text)
      {
         BrowserMessage msg{};
         auto           ec = glz::read_json(msg, msg_text);
         if (!ec)
         {
            dispatchMessage(msg);
         }
         else
         {
            SPDLOG_DEBUG("Headless web client message parse error: {}. Message: {}", glz::format_error(ec), msg_text);
            assert(false);
         }
      }
      else
      {
         SPDLOG_DEBUG("Unexpected opcode received in HeadlessClient::onMessage. Opcode: {}. Message: {}", enum_to_string(opcode), msg_text);
         assert(false);
      }
   }


   void HeadlessWebClient::onWebSocketError(std::error_code ec)
   {
      SPDLOG_DEBUG("HeadlessWebClient::onError - {} ({})", ec.message(), ec.value());
      assert(false);
   }


   void HeadlessWebClient::attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay)
   {
      // We need to use an HTTP GET to retrieve the WS endpoint and connect. This callback will run
      // on the io_context's thread.
      auto callback = [this, url, retries, retry_delay](HttpDownloader::HttpResult result) mutable
      {
         try
         {
            auto response = tasks::validateHttpResponse(result).response_body;
            SPDLOG_DEBUG("Got HTTP GET response from browser: {}", response);
            JsonPropMap props{};
            glz::ex::read_json(props, response);

            auto ws_url = std::get<std::string>(props[params::WS_DEBUG_URL]);
            m_ws_client.connect(ws_url);
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
               delayedExec(retry_delay,
                           [this, url = std::move(url), retries, retry_delay]() mutable
                           {
                              attemptWebsocketConnect(std::move(url), retries - 1U, retry_delay * 2);
                           });
            }
         }
      };

      m_http_client.getAsync(url, {}, std::move(callback));
   }


   void HeadlessWebClient::delayedExec(std::chrono::milliseconds delay, std::move_only_function<void()> func)
   {
      asio::co_spawn(
         *m_ctx,
         [this, func = std::move(func), delay]() mutable -> asio::awaitable<void>
         {
            asio::steady_timer timer(*m_ctx, delay);
            co_await timer.async_wait(asio::use_awaitable);
            func();
         },
         asio::detached);
   }


   void HeadlessWebClient::dispatchMessage(BrowserMessage msg)
   {
      uint32_t id = static_cast<uint32_t>(msg.id.value_or(0));
      if (id > 0)
      {
         // this is a command response.
         if (auto it = m_pending_requests.find(id); it != m_pending_requests.end())
         {
            auto handler = std::move(it->second);
            m_pending_requests.erase(it);
            if (handler)
            {
               // call completion handler from new async operation so that we can
               // unblock the websocket
               asio::post(*m_ctx,
                          [h = std::move(handler), msg = std::move(msg)]() mutable
                          {
                             h(std::move(msg));
                          });
            }
            else
            {
               SPDLOG_DEBUG(
                  "HeadlessWebClient - completion handler for command id {} was not in a valid state, async operation cannot be completed.",
                  id);
            }
         }
      }
      else
      {
         // It's a browser event. Figure out what to do with these. Probably a PageEventHandlers map that correlates session id with handler.
         SPDLOG_DEBUG("Browser event received method {} and params {}", msg.method.value_or(""), msg.params->str);
      }
   }

}   // namespace ctb::webclient


