#pragma once

#pragma once

#include <asio.hpp>
#include <asio/any_completion_handler.hpp>
#include <atomic>
#include <expected>
#include <glaze/glaze.hpp>
#include <glaze/net/websocket_client.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

// A simple structure to represent our outgoing command
struct CdpCommand
{
   int           id;
   std::string   method;
   glz::raw_json params;   // Omit if empty
};

class HeadlessWebClient
{
public:
   using ContextPtr =std::shared_ptr<asio::io_context>;

   HeadlessWebClient(ContextPtr io_ctx) : m_io_ctx(io_ctx), ws_client_(io_ctx)
   {
      setup_websocket_handlers();
   }

   // Connects the underlying websocket (You'd wrap this in a Sender too if desired)
   void connect(std::string_view url)
   {
      ws_client_.connect(url);
   }

   // ------------------------------------------------------------------------
   // THE BRIDGE: This function turns a WebSocket request into a stdexec Sender
   // ------------------------------------------------------------------------
   template<typename CompletionToken>
   auto async_send_command(const std::string& method, const std::string& params_json, CompletionToken&& token)
   {
      // 1. Generate a unique ID for this specific request
      int request_id = next_id_.fetch_add(1, std::memory_order_relaxed);

      // 2. Use Asio's async_initiate to bridge custom logic into a CompletionToken (like use_sender)
      // The signature void(std::string) defines what the resulting Sender will emit (the JSON string)
      return asio::async_initiate<CompletionToken, void(std::string)>(
         [this](auto handler, int id, std::string method, std::string params)
         {
            // A. Store the handler in our correlator map
            {
               std::lock_guard<std::mutex> lock(map_mutex_);
               // asio::any_completion_handler type-erases the complex stdexec receiver state
               pending_requests_[id] = std::move(handler);
            }

            // B. Format the JSON request (Using glaze)
            // Note: In production, you'd serialize the CdpCommand struct properly.
            std::string payload = R"({"id":)" + std::to_string(id) + R"(,"method":")" + method + R"(")";
            if (!params.empty())
            {
               payload += R"(,"params":)" + params;
            }
            payload += "}";

            // C. Send it out over the wire
            // (ws_client_.send is thread-safe via its internal mutex)
            ws_client_.send(payload);
         },
         token, request_id, method, params_json   // These arguments are forwarded into the lambda above
      );
   }

private:
   std::shared_ptr<asio::io_context> io_ctx_;
   glz::websocket_client             ws_client_;

   std::atomic<int> next_id_{ 1 };

   // The Correlator Map
   std::mutex map_mutex_;
   // Maps the CDP ID to an Asio completion handler that expects a string result
   std::unordered_map<int, asio::any_completion_handler<void(std::string)>> pending_requests_;

   void setup_websocket_handlers()
   {
      ws_client_.on_message(
         [this](std::string_view message, glz::ws_opcode opcode)
         {
            if (opcode != glz::ws_opcode::text) return;

            // 1. Quick and dirty parse to find the "id" (Use glz::read_json in production)
            // Assuming we parsed the JSON and extracted the ID and Result/Error
            int id = extract_id_from_json(message);

            if (id > 0)
            {
               // 2. This is a RESPONSE to a command we sent.
               asio::any_completion_handler<void(std::string)> handler;
               {
                  std::lock_guard<std::mutex> lock(map_mutex_);
                  auto                        it = pending_requests_.find(id);
                  if (it != pending_requests_.end())
                  {
                     handler = std::move(it->second);
                     pending_requests_.erase(it);
                  }
               }

               // 3. Fulfill the Sender!
               if (handler)
               {
                  // We post this back to the io_context to ensure the pipeline resumes
                  // cleanly without blocking the websocket's read loop.
                  asio::post(*io_ctx_,
                             [h = std::move(handler), msg = std::string(message)]() mutable
                             {
                                h(std::move(msg));   // This triggers stdexec::set_value!
                             });
               }
            }
            else
            {
               // 3. This is an unprompted EVENT (e.g., Page.loadEventFired)
               // Dispatch this to an Observer pattern / Event Bus
               handle_unprompted_event(message);
            }
         });
   }

   int extract_id_from_json(std::string_view msg)
   {
      // Stub: Use Glaze to parse the JSON and return the "id" integer.
      // Return 0 if "id" is missing (meaning it's an event).
      return 0;
   }

   void handle_unprompted_event(std::string_view msg)
   {
      // Here you would check if msg contains "Page.loadEventFired"
      // and notify whatever system is waiting for page loads.
   }


   auto my_pipeline = ex::just()
                    // 1. Hop to the IO thread
                    | ex::transfer(io_pool.get_scheduler())

                    // 2. Send the CDP Command and suspend the pipeline until the websocket replies
                    | ex::let_value(
                         [&cdp]()
                         {
                            return cdp.async_send_command("Page.navigate", R"({"url":"https://example.com"})", asioexec::use_sender);
                         })

                    // 3. Hop to the CPU thread to parse the heavy JSON response
                    | ex::transfer(cpu_pool.get_scheduler())
                    | ex::then(
                         [](std::string cdp_response)
                         {
                            // Parse the resulting frameId or error...
                            return parse_navigation_result(cdp_response);
                         });

   ex::start_detached(std::move(my_pipeline));

};
