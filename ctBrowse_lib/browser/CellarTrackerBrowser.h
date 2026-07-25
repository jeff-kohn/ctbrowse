#pragma once
#include "ctb/ctb.h"
#include "../async/senders.h"
#include <asio/io_context.hpp>
#include <exec/task.hpp>

namespace ctb
{
   class HeadlessBrowser;


   /// @brief Provides an async websocket interface for orchestrating a headless browser instance via Chrome Devtools Protocol.
   ///
   /// This class is meant to be thread-locked to a single ASIO thread for asynchronous operation, and does not protect data members
   /// from concurrent access.
   ///
   /// This class uses and returns stdexec-compatible asio coroutines that can be used from other coroutines or stdexec pipelines.
   /// The coroutine interface works better with the event-based websocket used for talking to the browser.
   class CellarTrackerBrowser
   {
   public:
      static constexpr int32_t           DEFAULT_WS_PORT   = 9222;
      static constexpr const char* const DEFAULT_DATA_DIR  = R"(%LOCALAPPDATA%\ctBrowse for Windows\WebView)";
      static constexpr const char* const DEFAULT_EDGE_PATH = R"(C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe)";

      using ContextPtr = std::shared_ptr<asio::io_context>;

      enum class Status : uint8_t
      {
         Unknown,
         Starting,
         Ready,
         ShuttingDown,
         Stopped,
      };


      /// @brief Initialize a CellarTrackerBrowser to run on the specified io_context
      /// @param io_ctx - thread-locked io_context
      ///
      /// This class does not protect its internal implementation from concurrent access since
      /// it is meant to run on a single-threaded context.
      CellarTrackerBrowser(ContextPtr io_ctx);


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
      auto status() const -> Status;


      /// @brief stdexec sender to download a label image from CT
      [[nodiscard]] senders::AnySender<senders::HttpFileContents> downloadLabel(uint64_t wine_id) noexcept(false);

      // needed in CPP for PIMPL
      ~CellarTrackerBrowser();

   private:
      indirect<HeadlessBrowser> m_browser;

      // private coroXXX methods are ASIO coroutines. Public methods that return stedexec senders
      // should only co_await these from inside an asSender() invocation, otherwise bad things can happen
      asio::awaitable<std::string> coroGetLabelImageUrl(const std::string& session_id) noexcept(false);
   };

}   // namespace ctb

