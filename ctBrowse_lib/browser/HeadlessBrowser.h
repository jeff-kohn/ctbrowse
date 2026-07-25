#pragma once

#include "ctb/ctb.h"
#include "utility_win32.h"
#include "async/HttpDownloader.h"
#include "browser/BrowserEvents.h"
#include "browser/webclient_schema.h"

#include <asio/any_completion_handler.hpp>
#include <asio/io_context.hpp>
#include <glaze/net/websocket_client.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace ctb
{


   /// @brief Provides an async websocket interface for orchestrating a headless browser instance via Chrome Devtools Protocol.
   ///
   /// This class is meant to be thread-locked to a single ASIO thread for asynchronous operation, and does not protect data members
   /// from concurrent access.
   ///
   /// This is a low-level class that implements asio coroutines that can be used as building blocks for more complex operations in a
   /// stdexec sender pipeline. The coroutine interface works better with the event-based websocket used for talking to the browser than
   /// stdexec pipelines.
   ///
   /// Most of the public methods will throw an exception on failure and are marked as noexcept(false). If a method returns an expected<>
   /// for retry loops where failure is not unexpected, it will be marked as noexcept().
   ///
   class HeadlessBrowser
   {
   public:
      using ContextPtr = std::shared_ptr<asio::io_context>;
      using WsClient   = glz::websocket_client;

      enum class Status : uint8_t
      {
         Unknown,
         Starting,
         Ready,
         ShuttingDown,
         Stopped,
      };


      /// @brief Initialize a HeadlessBrowser to run on the specified io_context
      /// @param io_ctx - thread-locked io_context
      ///
      /// This class does not protect its internal implementation from concurrent access since
      /// it is meant to run on a single-threaded context.
      HeadlessBrowser(ContextPtr io_ctx);
      ~HeadlessBrowser() noexcept;


      /// @brief start the browser process.
      ///
      /// Launches the headless browser, retrieves the WS endpoint via HTTP GET, and establishes initial WS connection.
      /// This method is safe to call from any thread, but should only be called once. Calling it again will throw an exception
      void start(std::string browser_path, std::string data_dir, int32_t port) noexcept(false);


      /// @brief stop accepting requests and attempt to shut down the websocket and browser connection cleanly.
      ///
      /// start() can safely be called once the stop operation is complete and status() returns Status::Stopped.
      void stop();


      /// @brief Returns the current status of the web client.
      Status status() const;


      /// @brief - RAII object that will tear down a browser tab/session on destruction.
      class Session
      {
      public:
         const std::string& sessionId() const
         {
            return m_session_id;
         }

         Session(Session&&) noexcept;
         Session& operator=(Session&&);
         ~Session() noexcept;

         Session()                          = delete;
         Session& operator=(const Session&) = delete;
         Session(const Session&)            = delete;

      private:
         friend class HeadlessBrowser;
         Session(HeadlessBrowser& browser, std::string session_id);

         std::string      m_session_id;
         HeadlessBrowser* m_browser{};
      };


      // result for coroCreateSession. Solves the problem of Session object not having default init or copy semantics. since ASIO/stdexec
      // async plumbing requires default-init in some paths for a result.
      using MaybeSession = std::optional<Session>;

      /// @brief creates a new target and session in the browser and returns the session so it can be used for additional commands on that target.
      [[nodiscard]] asio::awaitable<MaybeSession> coroCreateSession() noexcept(false);


      /// @brief retrieves a resource from the cache that was previously loaded by the specified frame
      /// @param session_id -
      /// @param frame_id 
      /// @param url 
      /// @return the requested resource's contents, which may or may not be base64-encoded.
      /// @throw ctb::Error if the contents could not be retrieved.
      [[nodiscard]] asio::awaitable<GetResourceResult> coroGetResource(const std::string& session_id,
                                                                       const std::string& frame_id,
                                                                       const std::string& url) noexcept(false);

      /// @brief Navigate to a web page and return once it is loaded.
      /// @param session_id - target session to use
      /// @param url  - url to navigate to
      /// @return  - the final URL that was loaded.
      /// @throw - ctb::Error if navigation or other error occurs
      [[nodiscard]] asio::awaitable<NavigateResult> coroNavigate(std::string session_id, std::string url) noexcept(false);


      using EvalReturnValue = std::expected<RuntimeEvalResult, ctb::Error>;

      /// @brief Use the DOM to evaluate a javascript expression
      /// @param session_id - target session to use
      /// @param expression - JS expression to evaluate
      /// @return 
      [[nodiscard]] asio::awaitable<EvalReturnValue> coroRuntimeEval(std::string session_id, std::string expression) noexcept;


      /// @brief close/destroy the specified session as a fire-and-forget async call
      void postCloseSession(std::string session_id) noexcept;


      /// @brief coroutine to send a command to the browser
      /// @param command    - the command name
      /// @param parameters - any parameters the command requires
      /// @param session_id - the session/target to use
      /// @return - asio awaitable
      [[nodiscard]] asio::awaitable<BrowserMessage> coroSendCommand(std::string command,
                                                                    JsonPropMap parameters,
                                                                    MaybeString session_id) noexcept(false);

 
      /// @brief coroutine to send a command to the browser
      /// @param command    - the command name
      /// @param parameters - any parameters the command requires
      /// @param session_id - the session/target to use
      /// @return - asio awaitable
      [[nodiscard]] asio::awaitable<BrowserMessage> coroSendCommand(std::string command,
                                                                    glz::raw_json parameters,
                                                                    MaybeString session_id) noexcept(false);

      /// @brief Fire-and-forget alternative to coroSendCommand()
      void postCommand(std::string command, JsonPropMap parameters, MaybeString session_id) noexcept;


      /// @brief retrieve an executor for the ASIO context this object is using.
      ///        can be used for co_spawn etc.
      auto getExecutor() const
      {
         return m_ctx->get_executor();
      }

   private:
      using CompletionHandler = asio::any_completion_handler<void(BrowserMessage)>;
      using CmdHandlerMap     = std::unordered_map<uint32_t, CompletionHandler>;        // map command id to completion handler
      using EventHandlerMap   = std::map<std::string, BrowserEventsPtr, std::less<>>;   // map session id to event channel

      static inline constexpr glz::opts JSON_OPTS{ .skip_null_members = true };

      alignas(std::hardware_destructive_interference_size) std::atomic<Status> m_status{ Status::Stopped };

      // ordering matters for this first group
      ContextPtr               m_ctx;
      WsClient                 m_ws_client;
      win32::ProcessJobHandles m_browser_handles{};

      CmdHandlerMap            m_command_handlers{};   // for responses from WS commands
      EventHandlerMap          m_event_handlers{};     // for events fired
      HttpDownloader           m_http_client;
      uint32_t                 m_next_id{ 1 };

      // WS event handling
      void setupHandlers();
      void onWebSocketOpen();
      void onWebSocketClose(glz::ws_close_code code, std::string_view reason);
      void onWebSocketMessage(std::string_view msg_text, glz::ws_opcode opcode);
      void onWebSocketError(std::error_code);

      // sub-coroutines called by the public methods.
      [[nodiscard]] asio::awaitable<std::string>    coroCreateTarget() noexcept(false);
      [[nodiscard]] asio::awaitable<Session>        coroAttachTarget(std::string target_id) noexcept(false);
      [[nodiscard]] asio::awaitable<void>           coroEnablePageEvents(std::string session_id) noexcept(false);
      [[nodiscard]] asio::awaitable<void>           coroEnableNetworkEvents(std::string session_id) noexcept(false);
      [[nodiscard]] asio::awaitable<BrowserMessage> coroAwaitEvent(std::string_view session_id) noexcept(false);

      // private implementation
      BrowserEventsPtr& getOrCreateEventHandler(std::string_view session_id);

      void attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay = 10ms);
      void coExec(std::move_only_function<void()> func);
      void delayedExec(std::chrono::milliseconds delay, std::move_only_function<void()> func);
      void dispatchMessage(BrowserMessage response);
      void subscribeEvent(const std::string& session_id, const std::string& event_name);
      void unSubscribeEvent(const std::string& session_id, const std::string& event_name);
      void unSubscribeAllEvents(const std::string& session_id);

      template<rng::input_range RngT> requires std::constructible_from<std::string, rng::range_value_t<RngT>>
      void subscribeEvents(const std::string& session_id, RngT&& event_names)
      {
         coExec(
            [this, session_id, event_names = std::forward<RngT>(event_names)] mutable
            {
               getOrCreateEventHandler(session_id)->subscribeEvents(event_names);
            });
      }

      template<rng::input_range RngT> requires std::constructible_from<std::string, rng::range_value_t<RngT>>
      void unSubscribeEvents(const std::string& session_id, RngT&& event_names)
      {
         coExec(
            [this, session_id, event_names = std::forward<RngT>(event_names)] mutable
            {
               getOrCreateEventHandler(session_id)->unSubscribeEvents(event_names);
            });
      }
   };


}   // namespace ctb


