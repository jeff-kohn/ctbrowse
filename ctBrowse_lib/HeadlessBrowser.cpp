#include "HeadlessBrowser.h"

#include "async_tasks.h"
#include "ctb/utility_templates.h"


#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include <glaze/glaze_exceptions.hpp>
#include <string_view>


namespace ctb::web
{
   using glz::ex::read_json;
   using glz::ex::write_json;

   constexpr auto MAX_CONNECT_RETRIES = 10;
   constexpr auto FMT_EDGE_HTTP_URL   = "http://127.0.0.1:{}/json/version";
   constexpr auto FMT_EDGE_ARGS       = "--headless=new --remote-debugging-address=127.0.0.1 "
                                        "--remote-debugging-port={} --disable-gpu "
                                        "--no-first-run --no-default-browser-check --disable-sync "
                                        "--user-data-dir=\"{}\""sv;

   namespace
   {
      inline void throwIfError(std::string_view command_name, const BrowserMessage& response)
      {
         if (response.error)
         {
            throw Error{ Error::Category::NetworkError, "{} failed. ({})", command_name, response.error->str };
         }
      }
   }   // namespace


   HeadlessBrowser::Session::Session(HeadlessBrowser& browser, std::string session_id)
      : m_session_id{ std::move(session_id) },
        m_browser{ &browser }
   {}


   HeadlessBrowser::Session::~Session() noexcept
   {
      try
      {
         // this object may not be valid if it was moved-from
         if (m_browser)
         {
            m_browser->postCloseSession(std::move(m_session_id));
         }
      }
      catch (...)
      {
         SPDLOG_DEBUG("HeadlessBrowser::Session::~Session caught exception closing session. {}", packageError().formattedMessage());
      }
   }


   HeadlessBrowser::Session::Session(Session&& other) noexcept : m_session_id{ std::move(other.m_session_id) }, m_browser{ other.m_browser }
   {
      other.m_browser = nullptr;
   }


   HeadlessBrowser::HeadlessBrowser(ContextPtr io_ctx) : m_ctx{ io_ctx }, m_ws_client{ io_ctx }
   {
      setupHandlers();
   }


   void HeadlessBrowser::start(std::string browser_path, std::string data_dir, int32_t port) noexcept(false)
   {
      if (m_status != Status::Stopped)
      {
         throw ctb::Error{ Error::Category::GeneralError, "HeadlessBrowser status is '{}', start() is not a valid operation.",
                           enum_to_string(m_status.load()) };
      }

      tryExpandEnvironmentVars(browser_path);
      tryExpandEnvironmentVars(data_dir);

      if (!fs::exists(browser_path))
      {
         throw ctb::Error{ Error::Category::FileError, "Couldn't launch headless web client, path \"{}\" does not exist.", browser_path };
      }
      if (!fs::exists(data_dir))
      {
         if (!createFolderPath(data_dir))
         {
            throw Error{ Error::Category::GeneralError,
                         "Couldn't launch headless web browser, data dir \"{}\" does not exist and could not be created.", data_dir };
         }
      }

      // First we need to launch the browser process.
      m_browser_handles = getValueOrThrow(win32::createProcessJob(browser_path, ctb::format(FMT_EDGE_ARGS, port, data_dir)));
      m_status.store(Status::Starting);

      // Establish the websocket connection
      auto url = ctb::format(FMT_EDGE_HTTP_URL, port);
      attemptWebsocketConnect(url, MAX_CONNECT_RETRIES);
   }


   void HeadlessBrowser::stop()
   {
      m_status.store(Status::ShuttingDown);
      postCommand(commands::CLOSE_BROWSER, {}, {});
      m_ws_client.close();
   }


   HeadlessBrowser::Status HeadlessBrowser::status() const
   {
      return m_status.load();
   }


   asio::awaitable<HeadlessBrowser::Session> HeadlessBrowser::coroCreateSession() noexcept(false)
   {
      // make sure we're on the io thread.
      co_await asio::dispatch(*m_ctx, asio::use_awaitable);

      std::string target_id = co_await coroCreateTarget();
      auto        session   = co_await coroAttachTarget(target_id);
      co_await coroEnablePage(session.sessionId());

      co_return session;
   }


   void HeadlessBrowser::postCloseSession(std::string session_id) noexcept
   {
      // clang-format off
      postCommand(commands::CLOSE_TARGET, { { params::TARGET_ID, std::move(session_id) } }, {});
      // clang-format on
   }


   [[nodiscard]] asio::awaitable<BrowserMessage> HeadlessBrowser::coroSendCommand(std::string command,
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
            m_command_handlers[msg.id.transform(to_unsigned).value_or(0U)] = std::move(handler);

            // serialize and send the message. response will come via on_message()
            auto json = write_json(msg);
            m_ws_client.send(json);
         },
         asio::use_awaitable, std::move(command), std::move(parameters), std::move(session_id));
   }


   void HeadlessBrowser::postCommand(std::string command, JsonPropMap parameters, MaybeString session_id) noexcept
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
                  SPDLOG_DEBUG("HeadlessBrowser::postCommand failed: {}", packageError().formattedMessage());
               }
            },
            asio::detached);
      }
      catch (...)   // NOLINT
      {
         SPDLOG_DEBUG("HeadlessBrowser::postCommand failed: {}", packageError().formattedMessage());
      }
   }


   void HeadlessBrowser::setupHandlers()
   {
      m_ws_client.on_open(std::bind_front(&HeadlessBrowser::onWebSocketOpen, this));
      m_ws_client.on_close(std::bind_front(&HeadlessBrowser::onWebSocketClose, this));
      m_ws_client.on_message(std::bind_front(&HeadlessBrowser::onWebSocketMessage, this));
      m_ws_client.on_error(std::bind_front(&HeadlessBrowser::onWebSocketError, this));
   }


   void HeadlessBrowser::onWebSocketOpen()
   {
      SPDLOG_DEBUG("HeadlessBrowser::onOpen - websocket connection established.");
      m_status.store(Status::Ready);
   }


   void HeadlessBrowser::onWebSocketClose(glz::ws_close_code code, std::string_view reason)
   {
      SPDLOG_DEBUG("HeadlessBrowser::onClose - code: {}, reason: '{}'", static_cast<uint16_t>(code), reason);
      m_status.store(Status::Stopped);
      m_browser_handles = {};   // kill the browser process.
   }


   void HeadlessBrowser::onWebSocketMessage(std::string_view msg_text, glz::ws_opcode opcode)
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
            SPDLOG_DEBUG("HeadlessBrowser message parse error: {}. Message: {}", glz::format_error(ec), msg_text);
            assert(false);
         }
      }
      else
      {
         SPDLOG_DEBUG(
            "Unexpected opcode received in HeadlessBrowser::onMessage. Opcode: {}. Message: {}", enum_to_string(opcode), msg_text);
         assert(false);
      }
   }


   void HeadlessBrowser::onWebSocketError(std::error_code ec)
   {
      SPDLOG_DEBUG("HeadlessBrowser::onError - {} ({})", ec.message(), ec.value());
      assert(false);
   }


   void HeadlessBrowser::attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay)
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
            read_json(props, response);

            auto ws_url = std::get<std::string>(props[params::WS_DEBUG_URL]);
            m_ws_client.connect(ws_url);
         }
         catch (...)
         {
            auto error = packageError();
            if (retries == 0)
            {
               m_status.store(Status::Stopped);
               m_browser_handles = {};
               SPDLOG_DEBUG("HeadlessBrowser couldn't establish connection with browser. {}", error.formattedMessage());
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


   [[nodiscard]] asio::awaitable<std::string> HeadlessBrowser::coroCreateTarget() noexcept(false)
   {
      // clang-format off
      auto response = co_await coroSendCommand(commands::CREATE_TARGET, JsonPropMap{ { params::TARGET_URL, params::ABOUT_BLANK } }, {});
      throwIfError(commands::CREATE_TARGET, response);
      // clang-format on

      auto result = read_json<CreateTargetResult>(response.result->str);
      SPDLOG_DEBUG("{} received targetId '{}'", commands::CREATE_TARGET, result.targetId);

      co_return result.targetId;
   }


   [[nodiscard]] asio::awaitable<HeadlessBrowser::Session> HeadlessBrowser::coroAttachTarget(std::string target_id) noexcept(false)
   {
      JsonPropMap params{
         { params::TARGET_ID, target_id },
         { params::FLATTEN,   true      }
      };

      auto response = co_await coroSendCommand(commands::ATTACH_TARGET, std::move(params), {});
      throwIfError(commands::ATTACH_TARGET, response);

      auto result = read_json<AttachTargetResult>(response.result->str);
      SPDLOG_DEBUG("{} received targetId '{}'", commands::ATTACH_TARGET, result.sessionId);

      co_return Session{ *this, std::move(result.sessionId) };
   }


   [[nodiscard]] asio::awaitable<void> HeadlessBrowser::coroEnablePage(std::string session_id) noexcept(false)
   {
      auto response = co_await coroSendCommand(commands::ENABLE_PAGE_EVENTS, {}, session_id);
      throwIfError(commands::ENABLE_PAGE_EVENTS, response);
      co_return;
   }


   void HeadlessBrowser::delayedExec(std::chrono::milliseconds delay, std::move_only_function<void()> func)
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


   void HeadlessBrowser::dispatchMessage(BrowserMessage msg)
   {
      uint32_t id = static_cast<uint32_t>(msg.id.value_or(0));
      if (id > 0)
      {
         // this is a command response.
         if (auto it = m_command_handlers.find(id); it != m_command_handlers.end())
         {
            auto handler = std::move(it->second);
            m_command_handlers.erase(it);
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
                  "HeadlessBrowser - completion handler for command id {} was not in a valid state, async operation cannot be completed.",
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

}   // namespace ctb::web


