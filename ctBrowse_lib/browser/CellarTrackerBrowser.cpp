#include "CellarTrackerBrowser.h"
#include "HeadlessBrowser.h"
#include "ctb/utility_chrono.h"
#include "ctb/utility_http.h"

#include <asio/dispatch.hpp>
#include <asio/this_coro.hpp>
#include <exec/asio/use_sender.hpp>
#include <fmt/chrono.h>
#include <utility>

namespace ctb
{
   using namespace senders;


   CellarTrackerBrowser::~CellarTrackerBrowser()
   {}


   CellarTrackerBrowser::CellarTrackerBrowser(ContextPtr io_ctx) : m_browser{ std::in_place, std::move(io_ctx) }
   {}


   void CellarTrackerBrowser::start(std::string browser_path, std::string data_dir, int32_t port) noexcept(false)
   {
      m_browser->start(browser_path, data_dir, port);
   }


   void CellarTrackerBrowser::stop()
   {
      m_browser->stop();
   }


   auto CellarTrackerBrowser::status() const -> Status
   {
      auto status = std::to_underlying(m_browser->status());
      return enum_cast<Status>(status).value_or(Status::Unknown);
   }


   AnySender<HttpFileContents> CellarTrackerBrowser::downloadLabel(uint64_t wine_id) noexcept(false)
   {

      // this lambda can directly await ASIO coroutines without having to mess with co_spawn and manually unwrapping
      // its return value.
      auto asio_coro = [this, wine_id]() -> asio::awaitable<HttpFileContents>
      {
         auto session = co_await m_browser->coroCreateSession();
         if (!session)
         {
            throw ctb::Error{ Error::Category::NetworkError, "Couldn't get browser session to download label for wine_id {}", wine_id };
         }

         auto nav_result           = co_await m_browser->coroNavigate(session->sessionId(), getWineDetailsUrl(wine_id));
         auto label_url            = co_await coroGetLabelImageUrl(session->sessionId());
         auto image_content_result = co_await m_browser->coroGetResource(session->sessionId(), nav_result.frame_id, label_url);

         co_return HttpFileContents{ .original_url   = move(label_url),
                                     .content        = move(image_content_result.content),
                                     .base64_encoded = image_content_result.base64Encoded };
      };

      // execute the lambda coro and return its result as a sender
      return senders::asSender<HttpFileContents>(m_browser->getExecutor(), asio_coro);
   }


   asio::awaitable<std::string> CellarTrackerBrowser::coroGetLabelImageUrl(const std::string& session_id) noexcept(false)
   {
      static const std::string expression =
         R"((() => { let el = document.getElementById('label_photo') || document.querySelector('.label_photo');
               if (!el) return 'ERROR: element not found'; if (el.tagName.toLowerCase() === 'img') return el.src;
               if (el.tagName.toLowerCase() === 'a') return el.href; let img = el.querySelector('img');
               if (img) return img.src; return 'ERROR: no image source found in element'; })())";

      constexpr auto RETRY_COUNT    = 6u;
      auto           retry_interval = 250ms;

      // page load is heavily async due to javascript and WAF, so we need to retry if we initially get error
      // due to page still loading.
      for (int i = RETRY_COUNT; i > 0; i--)
      {
         auto retval = co_await m_browser->coroRuntimeEval(session_id, expression);

         if (retval)
         {
            SPDLOG_DEBUG("HeadlessBrowser::coroRuntimeEval returned label URL '{}'", retval.value().result.value);
            co_return retval->result.value;
         }
         if (i > 1)
         {
            SPDLOG_DEBUG("HeadlessBrowser::coroRuntimeEval returned an error on try {}, retrying in {}", RETRY_COUNT - i, retry_interval);
            asio::steady_timer timer{ m_browser->getExecutor(), retry_interval };
            co_await timer.async_wait(asio::use_awaitable);
            retry_interval *= 2;
         }
         else
         {
            SPDLOG_DEBUG("coroRuntimeEval still returned an error after {} tries, throwing an error ({})", RETRY_COUNT,
                         retval.error().formattedMessage());

            auto html_result = co_await m_browser->coroRuntimeEval(session_id, "document.documentElement.outerHTML");
            SPDLOG_DEBUG("Page.navigate result: \r\n{}", html_result ? html_result->result.value : html_result.error().formattedMessage());
            throw std::move(retval.error());
         }
      }
   }


}   // namespace ctb


