#include "HeadlessBrowser.h"
#include "async/senders.h"
#include "json_serialization.h"

#include "ctb/utility_templates.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include <glaze/format/format_to.hpp>
#include <glaze/glaze_exceptions.hpp>
#include <string_view>


namespace ctb
{
   // easier to use exceptions than constantly checking error codes when we're writing coroutines
   using std::move;
   using std::string;
   using std::string_view;


   constexpr auto MAX_CONNECT_RETRIES = 10;
   constexpr auto FMT_EDGE_HTTP_URL   = "http://127.0.0.1:{}/json/version";

   constexpr auto FMT_EDGE_ARGS = "--headless=new "
                                  "--remote-debugging-address=127.0.0.1 "
                                  "--no-first-run --no-default-browser-check --disable-sync "
                                  "--disable-blink-features=AutomationControlled "
                                  "--window-size=1720,1010 "
                                  "--remote-debugging-port={} "
                                  "--user-data-dir=\"{}\""sv;
   namespace
   {
      inline void throwIfError(string_view command_name, const BrowserMessage& response) noexcept(false)
      {
         if (response.error)
         {
            throw Error{ Error::Category::NetworkError, "{} failed. ({})", command_name, response.error->str };
         }
      }


      // retrieves the result member as a JsonPropMap. If the prop map contains an "error" param it will
      // be throw as an Error.
      inline JsonPropMap getResultFromResponse(const BrowserMessage& msg) noexcept(false)
      {
         JsonPropMap results{};
         if (msg.result)
         {
            results = glz::ex::read_json<JsonPropMap>(msg.result->str);
            if (auto it = results.find(params::ERROR_TEXT); it != results.end())
            {
               throw ctb::Error{ asString(it->second), Error::Category::NetworkError };
            }
         }
         return results;
      }

      template<typename T>
      T getResultAs(const BrowserMessage& msg) noexcept(false)
      {
         if (msg.result)
         {
            T    t{};
            auto ec = glz::read_json(t, msg.result->str);
            if (ec)
            {
               throw Error{ Error::Category::ParseError, "Couldn't parse result from headless browser command response: {}",
                            glz::format_error(ec) };
            }
            return t;
         }
         throw Error{ "HeadlessBrowser couldn't parse empty result." };
      }

   }   // namespace


   HeadlessBrowser::Session::Session(HeadlessBrowser& browser, string session_id, std::string target_id)
      : m_browser{ &browser },
        m_session_id{ move(session_id) },
        m_target_id{ move(target_id) }
   {}


   HeadlessBrowser::Session::~Session() noexcept
   {
      try
      {
         // this object may not be valid if it was moved-from
         if (m_browser)
         {
            m_browser->postCloseSession(move(m_target_id));
         }
      }
      catch (...)
      {
         SPDLOG_DEBUG("HeadlessBrowser::Session::~Session caught exception closing session. {}", packageError().formattedMessage());
      }
   }


   HeadlessBrowser::Session::Session(Session&& other) noexcept
      : m_browser{ other.m_browser },
        m_session_id{ move(other.m_session_id) },
        m_target_id{ move(other.m_target_id) }
   {
      other.m_browser = nullptr;
   }


   HeadlessBrowser::Session& HeadlessBrowser::Session::operator=(Session&& other) noexcept
   {
      if (this != &other)
      {
         if (m_browser)
         {
            m_browser->postCloseSession(move(m_session_id));
         }
         m_session_id    = move(other.m_session_id);
         m_browser       = other.m_browser;
         other.m_browser = nullptr;
      }
      return *this;
   }


   HeadlessBrowser::HeadlessBrowser(ContextPtr io_ctx) : m_ctx{ io_ctx }, m_ws_client{ io_ctx }
   {
      setupHandlers();
   }


   HeadlessBrowser::~HeadlessBrowser() noexcept
   {
      try
      {
         stop();
      }
      catch (...)
      {
         SPDLOG_DEBUG("HeadlessBrowser caught exception shutting down: {}", packageError().formattedMessage());
      }
   }


   void HeadlessBrowser::start(string browser_path, string data_dir, int32_t port) noexcept(false)
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
      if (!fs::exists(data_dir) and !createFolderPath(data_dir))
      {
         throw Error{ Error::Category::GeneralError,
                      "Couldn't launch headless web browser, data dir \"{}\" does not exist and could not be created.", data_dir };
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
      if (m_status.load() == Status::ShuttingDown or m_status.load() == Status::Stopped) return;

      m_status.store(Status::ShuttingDown);
      postCommand(commands::CLOSE_BROWSER, {}, {});
      m_ws_client.close();
   }


   HeadlessBrowser::Status HeadlessBrowser::status() const
   {
      return m_status.load();
   }


   asio::awaitable<HeadlessBrowser::MaybeSession> HeadlessBrowser::coroCreateSession() noexcept(false)
   {
      // make sure we're on the io thread.
      co_await asio::dispatch(*m_ctx, asio::use_awaitable);

      string target_id = co_await coroCreateTarget();
      auto   session   = co_await coroAttachTarget(target_id);
      co_return session;
   }


   asio::awaitable<GetResourceResult> HeadlessBrowser::coroGetResource(std::string session_id,
                                                                       std::string frame_id,
                                                                       std::string url) noexcept(false)
   {
      co_await asio::dispatch(*m_ctx, asio::use_awaitable);

      JsonPropMap params{
         { params::FRAME_ID,   frame_id },
         { params::TARGET_URL, url      }
      };

      auto response = co_await coroSendCommand(commands::GET_RESOURCE_CONTENT, move(params), session_id);
      throwIfError(commands::GET_RESOURCE_CONTENT, response);

      co_return getResultAs<GetResourceResult>(response);
   }


   asio::awaitable<NavigateResult> HeadlessBrowser::coroNavigate(string session_id, string url) noexcept(false)
   {
      using namespace asio::experimental::awaitable_operators;

      co_await asio::dispatch(*m_ctx, asio::use_awaitable);

      subscribeEvents(session_id, std::array{ events::PAGE_LOAD, events::PAGE_FRAME_NAVIGATED, events::PAGE_DOM_LOADED });
      co_await coroEnablePageEvents(session_id);

      JsonPropMap stealth_params{
         { params::SOURCE, std::string{ params::STEALTH_NAVIGATOR_NEW_DOC_SCRIPT } }
      };
      //auto result = co_await coroSendCommand(commands::ADD_NEW_DOC_SCRIPT, stealth_params, session_id);
      auto result = co_await coroSendCommand(commands::SET_USER_AGENT, params::SET_USER_AGENT_PARAMS, session_id);

      // send the navigate command
      JsonPropMap params{
         { params::TARGET_URL, url }
      };
      auto nav_response = co_await coroSendCommand(commands::NAVIGATE_TO_URL, move(params), session_id);
      throwIfError("HeadlessBrowser::coroNavigate", nav_response);
      auto nav_result = getResultFromResponse(nav_response);


      // now wait for the events to fire that signify page has loaded enough for us to query the DOM.
      // PAGE_LOAD isn't sufficient because initial HTML may just be a small stub that run JS
      // to populate the page, we ne need PAGE_DOM_LOADED to signify that DOM tree has been build. We also
      // use the PAGE_FRAME_NAVIGATED event to get the frame_id which is needed for retrieving resources
      // through the DOM.
      NavigateResult retval{ .session_id = session_id, .frame_id = asString(nav_result[params::FRAME_ID]) };
      bool           page_loaded = false;
      bool           dom_loaded  = false;

      while (retval.url.empty() || !page_loaded || !dom_loaded)
      {
         auto event_msg = co_await coroAwaitEvent(session_id);
         if (!event_msg.method.has_value() or !event_msg.params.has_value())
            throw Error{ "HeadlessBrowser received invalid event message." };

         if (event_msg.method.value() == events::PAGE_FRAME_NAVIGATED)
         {
            auto fid       = glz::get_as_json<string_view, "/frame/id">(event_msg.params->str);
            auto frame_url = glz::get_as_json<string_view, "/frame/url">(event_msg.params->str);
            if (fid and retval.frame_id == *fid)
            {
               retval.url = frame_url.value_or("");
               if (retval.url != url)
               {
                  SPDLOG_DEBUG(
                     "Warning: Page.navigate navigated to url ({}) that doesn't match original requested url ({})", retval.url, url);
               }
            }
         }
         else if (event_msg.method.value() == events::PAGE_DOM_LOADED)
         {
            dom_loaded = true;
            SPDLOG_DEBUG("Page.domContentEventFired event received for url {}", retval.url);
         }
         else
         {
            // PAGE_LOAD
            page_loaded = true;
            SPDLOG_DEBUG("Page.loadEventFired event received for url {}", retval.url);
         }
      }

      co_return retval;
   }


   asio::awaitable<HeadlessBrowser::EvalReturnValue> HeadlessBrowser::coroRuntimeEval(string session_id, string expression) noexcept
   {
      co_await asio::dispatch( *m_ctx, asio::use_awaitable);

      JsonPropMap params{
         { params::RETURN_BY_VAL, true             },
         { params::EXPRESSION,    move(expression) }
      };

      auto response = co_await coroSendCommand(commands::RUNTIME_EVAL, move(params), session_id);
      throwIfError(commands::RUNTIME_EVAL, response);

      auto eval_result = getResultAs<RuntimeEvalResult>(response);
      if (eval_result.result.value.starts_with("ERROR"))
      {
         co_return std::unexpected{
            Error{ eval_result.result.value, Error::Category::NetworkError }
         };
      }
      co_return eval_result;
   }


   void HeadlessBrowser::postCloseSession(string session_id) noexcept
   {
      try
      {
         // clang-format off
      delayedExec(0ms, [this, session_id]
         {
            m_event_handlers.erase(session_id);
            postCommand(commands::CLOSE_TARGET, { { params::TARGET_ID, move(session_id) } }, {});
         });
         // clang-format on
      }
      catch (...)   // NOLINT
      {
         SPDLOG_DEBUG("HeadlessBrowser caught an exception posting close-session command: {}", packageError().formattedMessage());
      }
   }


   asio::awaitable<BrowserMessage> HeadlessBrowser::coroSendCommand(string      command,
                                                                    JsonPropMap parameters,
                                                                    MaybeString session_id) noexcept(false)
   {
      return coroSendCommand(move(command), glz::raw_json(glz::ex::write_json(parameters)), move(session_id));
   }


   asio::awaitable<BrowserMessage> HeadlessBrowser::coroSendCommand(string        command,
                                                                    glz::raw_json parameters,
                                                                    MaybeString   session_id) noexcept(false)
   {
      co_await asio::dispatch(*m_ctx, asio::use_awaitable);

      co_return co_await asio::async_initiate<decltype(asio::use_awaitable), void(BrowserMessage)>(
         [this](auto handler, string command, glz::raw_json parameters, MaybeString session_id)
         {
            BrowserCommand msg{ .id = m_next_id++, .method = move(command), .sessionId = move(session_id), .params = move(parameters) };

            // map the completion handler to id so that we can look it up and complete it when the browser message comes back
            m_command_handlers[msg.id.transform(to_unsigned).value_or(0U)] = move(handler);

            // serialize and send the message. response will come via on_message()
            auto json = glz::ex::write_json(msg);
            m_ws_client.send(json);
         },
         asio::use_awaitable,
         move(command),
         move(parameters),
         move(session_id));
   }


   void HeadlessBrowser::postCommand(string command, JsonPropMap parameters, MaybeString session_id) noexcept
   {
      try
      {
         asio::co_spawn(
            *m_ctx,
            [this, cmd = move(command), params = move(parameters),
             id = move(session_id)] -> asio::awaitable<void>   // NOLINT [cppcoreguidelines-avoid-capturing-lambda-coroutines]
            {
               try
               {
                  (void)co_await coroSendCommand(move(cmd), move(params), move(id));
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


   void HeadlessBrowser::subscribeEvent(const string& session_id, const string& event_name)
   {
      coExec(
         [this, session_id, event_name] mutable
         {
            getOrCreateEventHandler(session_id)->subscribeEvent(move(event_name));
         });
   }


   void HeadlessBrowser::unSubscribeEvent(const string& session_id, const string& event_name)
   {
      coExec(
         [this, session_id, event_name] mutable
         {
            getOrCreateEventHandler(session_id)->unSubscribeEvent(move(event_name));
         });
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


   void HeadlessBrowser::onWebSocketClose([[maybe_unused]] glz::ws_close_code code, [[maybe_unused]] string_view reason)
   {
      m_status.store(Status::Stopped);
      m_browser_handles = {};   // kill the browser process.
   }


   void HeadlessBrowser::onWebSocketMessage(string_view msg_text, glz::ws_opcode opcode)
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
         SPDLOG_DEBUG("Unexpected opcode received in HeadlessBrowser::onMessage. Opcode: {}. Message: {}",
                      enum_to_string(opcode),
                      msg_text);
         assert(false);
      }
   }


   void HeadlessBrowser::onWebSocketError([[maybe_unused]] std::error_code ec)
   {
      SPDLOG_DEBUG("HeadlessBrowser::onError - {} ({})", ec.message(), ec.value());
   }


   BrowserEventsPtr& HeadlessBrowser::getOrCreateEventHandler(string_view session_id)
   {
      if (auto it = m_event_handlers.find(session_id); it != m_event_handlers.end())
      {
         return it->second;
      }
      // clang-format off

      return m_event_handlers.try_emplace(
         string{ session_id },
         std::make_shared<BrowserEvents>(m_ctx->get_executor(),
         string{ session_id })).first->second;

   }   // clang-format on


   void HeadlessBrowser::attemptWebsocketConnect(string url, uint8_t retries, std::chrono::milliseconds retry_delay)
   {
      // We need to use an HTTP GET to retrieve the WS endpoint and connect. This callback will run
      // on the io_context's thread.
      auto callback = [this, url, retries, retry_delay](HttpDownloader::HttpResult result) mutable
      {
         try
         {
            auto response = senders::validateHttpResponse(result).response_body;
            SPDLOG_DEBUG("Got HTTP GET response from browser: {}", response);
            JsonPropMap props{};
            glz::ex::read_json(props, response);

            auto ws_url = std::get<string>(props[params::WS_DEBUG_URL]);
            m_ws_client.connect(ws_url);
         }
         catch (...)
         {
            auto error = packageError();
            if (retries == 0)
            {
               m_status.store(Status::Stopped);
               m_browser_handles = {};   // kills edge if process hung
               SPDLOG_DEBUG("HeadlessBrowser couldn't establish connection with browser. {}", error.formattedMessage());
            }
            else
            {
               SPDLOG_DEBUG("Browser not ready, retrying in {}... ({} retries left)", retry_delay, retries);
               delayedExec(retry_delay,
                           [this, url = move(url), retries, retry_delay]() mutable
                           {
                              attemptWebsocketConnect(move(url), retries - 1U, retry_delay * 2);
                           });
            }
         }
      };

      m_http_client.getAsync(url, {}, move(callback));
   }


   asio::awaitable<string> HeadlessBrowser::coroCreateTarget() noexcept(false)
   {
      // clang-format off
      auto response = co_await coroSendCommand(commands::CREATE_TARGET, JsonPropMap{ { params::TARGET_URL, std::string{ params::ABOUT_BLANK } } }, {});
      throwIfError(commands::CREATE_TARGET, response);
      // clang-format on

      auto result = glz::ex::read_json<CreateTargetResult>(response.result.value_or({}).str);
      SPDLOG_DEBUG("{} received targetId '{}'", commands::CREATE_TARGET, result.targetId);

      co_return result.targetId;
   }


   asio::awaitable<HeadlessBrowser::Session> HeadlessBrowser::coroAttachTarget(string target_id) noexcept(false)
   {
      JsonPropMap params{
         { params::TARGET_ID, target_id },
         { params::FLATTEN,   true      }
      };

      auto response = co_await coroSendCommand(commands::ATTACH_TARGET, move(params), {});
      throwIfError(commands::ATTACH_TARGET, response);

      auto result = glz::ex::read_json<AttachTargetResult>(response.result.value_or({}).str);
      SPDLOG_DEBUG("{} received targetId '{}'", commands::ATTACH_TARGET, result.sessionId);

      co_return Session{ *this, move(result.sessionId), move(target_id) };
   }


   asio::awaitable<void> HeadlessBrowser::coroEnablePageEvents(string session_id) noexcept(false)
   {
      auto response = co_await coroSendCommand(commands::ENABLE_PAGE_EVENTS, JsonPropMap{}, session_id);
      throwIfError(commands::ENABLE_PAGE_EVENTS, response);
      co_return;
   }


   asio::awaitable<void> HeadlessBrowser::coroEnableNetworkEvents(string session_id) noexcept(false)
   {
      auto response = co_await coroSendCommand(commands::ENABLE_NETWORK_EVENTS, JsonPropMap{}, session_id);
      throwIfError(commands::ENABLE_NETWORK_EVENTS, response);
      co_return;
   }


   asio::awaitable<BrowserMessage> HeadlessBrowser::coroAwaitEvent(string_view session_id) noexcept(false)
   {
      if (auto it = m_event_handlers.find(session_id); it != m_event_handlers.end())
      {
         co_return co_await it->second->coroAwaitEvent();
      }
      throw ctb::Error{};
   }


   void HeadlessBrowser::coExec(std::move_only_function<void()> func)
   {
      asio::co_spawn(
         *m_ctx,
         [func = move(func)]() mutable -> asio::awaitable<void>   // NOLINT [cppcoreguidelines-avoid-capturing-lambda-coroutines]
         {
            func();
            co_return;
         },
         asio::detached);
   }


   void HeadlessBrowser::delayedExec(std::chrono::milliseconds delay, std::move_only_function<void()> func)
   {
      asio::co_spawn(
         *m_ctx,
         [this, func = move(func),
          delay]() mutable -> asio::awaitable<void>   // NOLINT [cppcoreguidelines-avoid-capturing-lambda-coroutines]
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
            auto handler = move(it->second);
            m_command_handlers.erase(it);
            if (handler)
            {
               // call completion handler from new async operation so that we can unblock the websocket
               asio::post(*m_ctx,
                          [h = move(handler), msg = move(msg)]() mutable
                          {
                             h(move(msg));
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
         // This is a browser event. Push it to the appropriate handling channel if we have one, discard otherwise.
         if (msg.sessionId.has_value())
         {
            if (auto it = m_event_handlers.find(msg.sessionId.value()); it != m_event_handlers.end())
            {
               it->second->postEvent(move(msg));
            }
         }
         else
         {
            SPDLOG_DEBUG("HeadlessBrowser::DispatchMessage received event {} with no registered handlers.", msg.method.value_or(""));
         }
      }
   }
}   // namespace ctb


