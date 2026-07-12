#include "CellarTrackerBrowser.h"
#include "HeadlessBrowser.h"
#include "senders.h"
#include "ctb/utility_http.h"

#include <asio/dispatch.hpp>
#include <exec/asio/use_sender.hpp>

#include <optional>
#include <utility>

namespace ctb::web
{

   CallarTrackerBrowser::~CallarTrackerBrowser()
   {}


   CallarTrackerBrowser::CallarTrackerBrowser(ContextPtr io_ctx) : m_browser{ std::in_place, std::move(io_ctx) }
   {}


   void CallarTrackerBrowser::start(std::string browser_path, std::string data_dir, int32_t port) noexcept(false)
   {
      m_browser->start(browser_path, data_dir, port);
   }


   void CallarTrackerBrowser::stop()
   {
      m_browser->stop();
   }


   auto CallarTrackerBrowser::status() const -> Status
   {
      auto status = std::to_underlying(m_browser->status());
      return enum_cast<Status>(status).value_or(Status::Unknown);
   }


   [[nodiscard]] exec::task<Buffer> CallarTrackerBrowser::sndDownloadLabel(uint64_t wine_id) noexcept(false)
   {
      // this lambda can directly await ASIO coroutines without having to mess with co_spawn and manually unwrapping
      // its return value.
      auto asio_coro = [this, wine_id]() -> asio::awaitable<Buffer>
      {
         auto session = co_await m_browser->coroCreateSession();
         if (!session)
         {
            throw ctb::Error{ Error::Category::NetworkError, "Couldn't get browser session for label download for wine_id {}", wine_id };
         }
         auto request_url = ctb::getWineDetailsUrl(wine_id);

         auto actual_url = co_await m_browser->coroNavigate(session->sessionId(), request_url);
         if (request_url != actual_url)
         {
            throw Error{ Error::Category::NetworkError, "Could not navigate to {}, actual page was {}", request_url, actual_url };
         }
         co_return Buffer{};
      };

      co_return co_await tasks::asioAwait(m_browser->getExecutor(), asio_coro());
   }


}   // namespace ctb::web

