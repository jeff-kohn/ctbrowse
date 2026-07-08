#pragma once
#include "ctb/ctb.h"
#include "utility_win32.h"
#include "webclient_schema.h"

#include "ctb/HttpDownloader.h"

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


namespace ctb::webclient
{


   /// @brief Provides an async websocket interface for orchestrating a headless browser instance via Chrome Devtools Protocol.
   ///
   /// This class is meant to be thread-locked to a single ASIO thread for asynchronous operation, and does not protect data members
   /// from concurrent access.
   ///
   /// This class uses and returns stdexec-compatible asio coroutines that can be used from other coroutines or stdexec pipelines.
   /// The coroutine interface works better with the event-based websocket used for talking to the browser.
   class HeadlessWebClient
   {
   public:
      static constexpr int32_t           DEFAULT_WS_PORT   = 9222;
      static constexpr const char* const DEFAULT_DATA_DIR  = R"(%LOCALAPPDATA%\ctBrowse for Windows\WebView)";
      static constexpr const char* const DEFAULT_EDGE_PATH = R"(C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe)";

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


      /// @brief Initialize a HeadlessWebClient to run on the specified io_context
      /// @param io_ctx - thread-locked io_context
      ///
      /// This class does not protect its internal implementation from concurrent access since
      /// it is meant to run on a single-threaded context.
      HeadlessWebClient(ContextPtr io_ctx);


      /// @brief start the browser process.
      ///
      /// Launches the headless browser, retrieves the WS endpoint via HTTP GET, and establishes initial WS connection.
      /// This method is safe to call from any thread, but should only be called once. Calling it again will throw an exception
      void start(std::string browser_path = DEFAULT_EDGE_PATH,
                 std::string data_dir     = DEFAULT_DATA_DIR,
                 int32_t     port         = DEFAULT_WS_PORT) noexcept(false);


      /// @brief stop accepting requests and attempt to shut down the websocket and browser connection cleanly.
      ///
      /// start() can safely be called once the stop operation is complete and status() returns Status::Stopped.
      void stop();


      /// @brief Returns the current status of the web client.
      Status status() const;


      /// @brief - RAII object that will tear down a browser tab/session on destruction.
      class TargetSession
      {
      public:
         const std::string& sessionId() const
         {
            return m_session_id;
         }

         ~TargetSession() noexcept;
         TargetSession(TargetSession&&) noexcept;
         TargetSession()                                = delete;
         TargetSession& operator=(TargetSession&&)      = delete;
         TargetSession& operator=(const TargetSession&) = delete;
         TargetSession(const TargetSession&)            = delete;

      private:
         friend class HeadlessWebClient;

         std::string        m_session_id;
         HeadlessWebClient* m_web_client{};

         TargetSession(HeadlessWebClient& client, std::string session_id) : m_session_id{ std::move(session_id) }, m_web_client{ &client }
         {}
      };


      /// @brief creates a new target and session in the browser and returns the session so it can be used for additional commands on that target.
      [[nodiscard]] asio::awaitable<TargetSession> coroCreateSession() noexcept(false);


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


   private:
      // map browser command-id to completion handlerso
      using RequestMap = std::unordered_map<uint32_t, asio::any_completion_handler<void(BrowserMessage)>>;

      static inline constexpr glz::opts JSON_OPTS{ .skip_null_members = true };

      alignas(std::hardware_destructive_interference_size) std::atomic<Status> m_client_status{ Status::Stopped };

      ContextPtr               m_ctx;
      win32::ProcessJobHandles m_browser_handles{};
      HttpDownloader           m_http_client;
      uint32_t                 m_next_id{ 1 };
      RequestMap               m_pending_requests{};
      WsClient                 m_ws_client;

      // Event handling setup
      void setupHandlers();
      void onWebSocketOpen();
      void onWebSocketClose(glz::ws_close_code code, std::string_view reason);
      void onWebSocketMessage(std::string_view msg_text, glz::ws_opcode opcode);
      void onWebSocketError(std::error_code);

      // private implementation
      void attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay = 10ms);

      [[nodiscard]] asio::awaitable<BrowserMessage> coroSendCommand(std::string command,
                                                                    std::string param_json,
                                                                    MaybeString session_id) noexcept(false);

      /// @brief runs a callable on the io_context as a fire-and-forget operation with a timed delay
      /// @param delay - timer value to use for delay before execution
      /// @param func  - the callable to execute
      void delayedExec(std::chrono::milliseconds delay, std::move_only_function<void()> func);

      /// @brief handles messages sent from the browser in response to a command by routing them back to the appropriate completion handler
      void dispatchMessage(BrowserMessage response);
   };


   //auto my_pipeline = ex::just()
   //                 // 1. Hop to the IO thread
   //                 | ex::transfer(io_pool.get_scheduler())

   //                 // 2. Send the CDP Command and suspend the pipeline until the websocket replies
   //                 | ex::let_value(
   //                      [&cdp]()
   //                      {
   //                         return cdp.async_send_command("Page.navigate", R"({"url":"https://example.com"})", asioexec::use_sender);
   //                      })

   //                 // 3. Hop to the CPU thread to parse the heavy JSON response
   //                 | ex::transfer(cpu_pool.get_scheduler())
   //                 | ex::then(
   //                      [](std::string cdp_response)
   //                      {
   //                         // Parse the resulting frameId or error...
   //                         return parse_navigation_result(cdp_response);
   //                      });

   //ex::start_detached(std::move(my_pipeline));


}   // namespace ctb::webclient
