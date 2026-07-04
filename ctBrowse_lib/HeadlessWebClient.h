#pragma once
#include "ctb/HttpDownloader.h"
#include "ctb/ctb.h"
#include "utility_win32.h"

#include <asio/any_completion_handler.hpp>
#include <asio/io_context.hpp>
#include <glaze/net/websocket_client.hpp>

#include <atomic>
#include <chrono>
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>


namespace ctb::web
{
   using NullableString    = std::optional<std::string>;
   using StringMap         = std::map<std::string, std::string>;
   using NullableStringMap = std::optional<StringMap>;


   struct BrowserMessage
   {
      std::string       method;
      NullableInt       id;
      NullableString    sessionId;
      NullableStringMap params;
   };

   //struct BrowserEvent
   //{
   //   std::string       method;
   //   NullableString    sessionId;
   //   NullableStringMap params;
   //};


   /// @brief Provides an async websocket interface for orchestrating a headless browser instance via Chrome Devtools Protocol.
   ///
   /// This class is meant to be thread-locked to a single ASIO thread for asynchronous operation, and does not protect data members
   /// from concurrent access.
   class HeadlessWebClient
   {
   public:
      static constexpr int32_t           DEFAULT_WS_PORT   = 9222;
      static constexpr const char* const DEFAULT_DATA_DIR  = R"(%LOCALAPPDATA%\ctBrowse for Windows\WebView)";
      static constexpr const char* const DEFAULT_EDGE_PATH = R"(C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe)";

      enum class Status : uint8_t
      {
         Unknown,
         Starting,
         Ready,
         ShuttingDown,
         Stopped,
      };

      using ContextPtr = std::shared_ptr<asio::io_context>;
      using TextResult = std::expected<std::string, ctb::Error>;
      using WsClient   = glz::websocket_client;


      /// @brief Initialize a HeadlessWebClient to run on the specified io_context
      /// @param io_ctx - thread-locked io_context
      ///
      /// This class does not protect its internal implementation from concurrent access since
      /// it is meant to run on a single-threaded context.
      HeadlessWebClient(ContextPtr io_ctx);


      /// @brief start the browser process.
      ///
      /// Launches the headless browser, retrieves the WS endpoint via HTTP get and establishes initial WS connection.
      /// This method is safe to call from any thread, but should only be called once. Calling it again will throw an exception
      void start(std::string browser_path = DEFAULT_EDGE_PATH,
                 std::string data_dir     = DEFAULT_DATA_DIR,
                 int32_t     port         = DEFAULT_WS_PORT) noexcept(false);


      /// @brief Returns the current status of the web client.
      auto status() const -> Status
      {
         return m_client_status.load();
      }

      // ------------------------------------------------------------------------
      // THE BRIDGE: This function turns a WebSocket request into a stdexec Sender
      // ------------------------------------------------------------------------
      //template<typename CompletionToken>
      //auto async_send_command(const std::string& method, const std::string& params_json, CompletionToken&& token)
      //{
      //   // 1. Generate a unique ID for this specific request
      //   int request_id = next_id_.fetch_add(1, std::memory_order_relaxed);

      //   // 2. Use Asio's async_initiate to bridge custom logic into a CompletionToken (like use_sender)
      //   // The signature void(std::string) defines what the resulting Sender will emit (the JSON string)
      //   return asio::async_initiate<CompletionToken, void(std::string)>(
      //      [this](auto handler, int id, std::string method, std::string params)
      //      {
      //         // A. Store the handler in our correlator map
      //         {
      //            std::lock_guard<std::mutex> lock(map_mutex_);
      //            // asio::any_completion_handler type-erases the complex stdexec receiver state
      //            pending_requests_[id] = std::move(handler);
      //         }

      //         // B. Format the JSON request (Using glaze)
      //         // Note: In production, you'd serialize the CdpCommand struct properly.
      //         std::string payload = R"({"id":)" + std::to_string(id) + R"(,"method":")" + method + R"(")";
      //         if (!params.empty())
      //         {
      //            payload += R"(,"params":)" + params;
      //         }
      //         payload += "}";

      //         // C. Send it out over the wire
      //         // (ws_client_.send is thread-safe via its internal mutex)
      //         ws_client_.send(payload);
      //      },
      //      token, request_id, method, params_json   // These arguments are forwarded into the lambda above
      //   );
      //}

   private:
      using RequestMap = std::unordered_map<int, asio::any_completion_handler<void(std::string)>>;

      alignas(std::hardware_destructive_interference_size) std::atomic<Status> m_client_status{ Status::Stopped };

      ContextPtr               m_ctx;
      win32::ProcessJobHandles m_browser_handles{};
      HttpDownloader           m_http_client;
      int                      m_next_id{ 1 };
      RequestMap               m_pending_requests{};
      WsClient                 m_ws_client;


      // Event handling setup
      void setupHandlers();
      void onOpen();
      void onClose(glz::ws_close_code code, std::string_view reason);
      void onMessage(std::string_view, glz::ws_opcode);
      void onError(std::error_code);

      void attemptWebsocketConnect(std::string url, uint8_t retries, std::chrono::milliseconds retry_delay = 10ms);
      void runWithDelay(std::chrono::milliseconds delay, std::move_only_function<void()> func);

      //void setup_websocket_handlers()
      //{
      //ws_client_.on_message(
      //   [this](std::string_view message, glz::ws_opcode opcode)
      //   {
      //      if (opcode != glz::ws_opcode::text) return;

      //      // 1. Quick and dirty parse to find the "id" (Use glz::read_json in production)
      //      // Assuming we parsed the JSON and extracted the ID and Result/Error
      //      int id = extract_id_from_json(message);

      //      if (id > 0)
      //      {
      //         // 2. This is a RESPONSE to a command we sent.
      //         asio::any_completion_handler<void(std::string)> handler;
      //         {
      //            std::lock_guard<std::mutex> lock(map_mutex_);
      //            auto                        it = pending_requests_.find(id);
      //            if (it != pending_requests_.end())
      //            {
      //               handler = std::move(it->second);
      //               pending_requests_.erase(it);
      //            }
      //         }

      //         // 3. Fulfill the Sender!
      //         if (handler)
      //         {
      //            // We post this back to the io_context to ensure the pipeline resumes
      //            // cleanly without blocking the websocket's read loop.
      //            asio::post(*io_ctx_,
      //                       [h = std::move(handler), msg = std::string(message)]() mutable
      //                       {
      //                          h(std::move(msg));   // This triggers stdexec::set_value!
      //                       });
      //         }
      //      }
      //      else
      //      {
      //         // 3. This is an unprompted EVENT (e.g., Page.loadEventFired)
      //         // Dispatch this to an Observer pattern / Event Bus
      //         handle_unprompted_event(message);
      //      }
      //   });


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
   };

}   // namespace ctb::web
