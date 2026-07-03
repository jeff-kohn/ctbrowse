#include "HeadlessWebClient.h"
#include "ctb/utility_templates.h"
#include "ctb/tasks/async_tasks.h"
#include <string_view>


namespace ctb::web
{

   constexpr auto FMT_EDGE_HTTP_URL = "http://127.0.0.1:{}/json/version";
   constexpr auto FMT_EDGE_ARGS     = "--headless --no-first-run --no-default-browser-check --disable-sync "
                                      "--remote-debugging-port={} --disable-gpu "
                                      "--user-data-dir=\"{}\""sv;


   HeadlessWebClient::HeadlessWebClient(ContextPtr io_ctx) : m_ctx{ io_ctx }, m_ws_client{ io_ctx }
   {
      //setupHandlers();
   }


   void HeadlessWebClient::start(std::string_view browser_path, std::string_view data_dir, int32_t port) noexcept(false)
   {
      if (m_client_status != ClientStatus::Stopped)
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
      m_client_status.store(ClientStatus::Starting);

      // Next we need to use an HTTP GET to retrieve the WS endpoint and connect. This callback will run on the io_context's
      // thread 
      auto callback = [this](HttpDownloader::HttpResult result)
      {
         try
         {
            auto response = tasks::validateHttpResponse(result).response_body;
            SPDLOG_DEBUG("Got HTTP GET response from browser: {}", response);
            
         }
         catch(...)
         {
            // we didn't get an URL back or couldn't send the ws_connect, so we should kill the process
            // and set our status to stopped.
            m_client_status.store(ClientStatus::Stopped);
            m_browser_handles = {};

            SPDLOG_DEBUG("HeadlessWebClient couldn't establish connection with browser. {}", packageError().formattedMessage());
         }
      };

      auto url = ctb::format(FMT_EDGE_HTTP_URL, port);
      m_http_client.getAsync(url, {}, std::move(callback));
   }

}   // namespace ctb::web


