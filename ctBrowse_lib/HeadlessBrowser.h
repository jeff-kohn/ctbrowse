#pragma once

#include "ctb/ctb.h"
#include "utility_win32.h"
#include "webclient_schema.h"

#include "HttpDownloader.h"

#include <asio/any_completion_handler.hpp>
#include <asio/io_context.hpp>
#include <glaze/net/websocket_client.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace ctb::web
{


   /// @brief Provides an async websocket interface for orchestrating a headless browser instance via Chrome Devtools Protocol.
   ///
   /// This class is meant to be thread-locked to a single ASIO thread for asynchronous operation, and does not protect data members
   /// from concurrent access.
   ///
   /// This class uses and returns stdexec-compatible asio coroutines that can be used from other coroutines or stdexec pipelines.
   /// The coroutine interface works better with the event-based websocket used for talking to the browser.
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


      // result to for coroCreateSession. Solves the problem of Session object not having default init or copy semantics. since ASIO/stdexec
      // async plumbing requires default-init in some paths for a result.
      using MaybeSession = std::optional<Session>;

      /// @brief creates a new target and session in the browser and returns the session so it can be used for additional commands on that target.
      [[nodiscard]] asio::awaitable<MaybeSession> coroCreateSession() noexcept(false);


      /// @brief Navigate to a web page and return once it is loaded.
      /// @param session_id - target session to use
      /// @param url  - url to navigate to
      /// @return  - the final URL that was loaded.
      /// @throw - ctb::Error if navigation or other error occurs
      [[nodiscard]] asio::awaitable<std::string> coroNavigate(std::string session_id, std::string url);


      /// @brief close/destroy the specified session as a fire-and-forget async call
      void postCloseSession(std::string session_id) noexcept;


      /// @brief coroutine to send a command to the browser
      /// @param session_id - the session/target to use
      /// @param command    - the command name
      /// @param parameters - any parameters the command requires
      /// @return - asio awaitable
      [[nodiscard]] asio::awaitable<BrowserMessage> coroSendCommand(std::string command,
                                                                    JsonPropMap parameters,
                                                                    MaybeString session_id) noexcept(false);

      /// @brief Fire-and-forget alternative to coroSendCommand()
      void postCommand(std::string command, JsonPropMap parameters, MaybeString session_id) noexcept;


      /// @brief retreive an executor for the ASIO context this object is using.
      ///        can be used for co_spawn etc.
      auto getExecutor() const
      {
         return m_ctx->get_executor();
      }

   private:
      // map browser command-id to completion handlers
      using CompletionHandler = asio::any_completion_handler<void(BrowserMessage)>;
      using HandlerMap        = std::unordered_map<uint32_t, CompletionHandler>;

      static inline constexpr glz::opts JSON_OPTS{ .skip_null_members = true };

      alignas(std::hardware_destructive_interference_size) std::atomic<Status> m_status{ Status::Stopped };

      win32::ProcessJobHandles m_browser_handles{};
      ContextPtr               m_ctx;
      HttpDownloader           m_http_client;
      uint32_t                 m_next_id{ 1 };
      HandlerMap               m_command_handlers{}; // for responses from WS commands
      HandlerMap               m_event_handlers{};   // for events fired
      WsClient                 m_ws_client;

      // WS event handling
      void setupHandlers();
      void onWebSocketOpen();
      void onWebSocketClose(glz::ws_close_code code, std::string_view reason);
      void onWebSocketMessage(std::string_view msg_text, glz::ws_opcode opcode);
      void onWebSocketError(std::error_code);

      // private implementation
      void attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay = 10ms);

      // sub-coroutines called by the public methods.
      [[nodiscard]] asio::awaitable<std::string> coroCreateTarget() noexcept(false);
      [[nodiscard]] asio::awaitable<Session>     coroAttachTarget(std::string target_id) noexcept(false);
      [[nodiscard]] asio::awaitable<void>        coroEnablePage(std::string session_id) noexcept(false);

      /// @brief runs a callable on the io_context as a fire-and-forget operation with a timed delay
      /// @param delay - timer value to use for delay before execution
      /// @param func  - the callable to execute
      void delayedExec(std::chrono::milliseconds delay, std::move_only_function<void()> func);

      /// @brief handles messages sent from the browser in response to a command by routing them back to the appropriate completion handler
      void dispatchMessage(BrowserMessage response);
   };


}   // namespace ctb::web

