#include "CellarTrackerBrowser.h"
#include "HeadlessBrowser.h"

#include <optional>
#include <utility>

namespace ctb::web
{

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


   asio::awaitable<Buffer> CallarTrackerBrowser::coroDownloadLabel(int64_t wine_id) noexcept(false)
   {
      return asio::awaitable<Buffer>();
   }


}

