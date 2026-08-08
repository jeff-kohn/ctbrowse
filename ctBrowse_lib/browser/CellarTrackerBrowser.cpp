#include "CellarTrackerBrowser.h"
#include "HeadlessBrowser.h"
#include "ctb/utility_chrono.h"
#include "ctb/utility_http.h"

#include <asio/dispatch.hpp>
#include <asio/this_coro.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <exec/asio/use_sender.hpp>
#include <fmt/chrono.h>

#include <utility>

namespace ctb
{
   using namespace senders;
   using namespace asio;

   inline auto sndNotReadyError(CellarTrackerBrowser::Status status)
   {
      return stdexec::just_error(std::make_exception_ptr(
         ctb::Error{ Error::Category::GeneralError, constants::FMT_ERROR_HEADLESS_BROWSER_INVALID_STATUS, status }));
   }

   CellarTrackerBrowser::~CellarTrackerBrowser() = default;


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


   AnySender<HttpFileContents> CellarTrackerBrowser::sndDownloadLabel(uint64_t wine_id) noexcept(false)
   {
      if (status() != Status::Ready) return sndNotReadyError(status());

      // this lambda can directly await ASIO coroutines without having to mess with co_spawn and manually unwrapping
      // its return value.
      // NOLINTNEXTLINE [cppcoreguidelines-avoid-capturing-lambda-coroutines] this ptr is safe here
      auto asio_coro = [this, wine_id]() -> awaitable<HttpFileContents>
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
      return asSender<HttpFileContents>(m_browser->getExecutor(), asio_coro);
   }


   template<DurationType DurationT>
   awaitable<CellarTrackerBrowser::ExpectedEvalResult> CellarTrackerBrowser::coroEvalWithRetry(std::string session_id,
                                                                                               std::string source_js,
                                                                                               uint16_t    num_retries,
                                                                                               DurationT   retry_delay,
                                                                                               double      backoff_factor) noexcept
   {
      for (uint16_t retry = 0;; ++retry)
      {
         auto retval = co_await m_browser->coroRuntimeEval(session_id, source_js);

         if (retval)
         {
            SPDLOG_DEBUG("CellarTrackerBrowser::coroEvalWithRetry returning result on attempt {}", retry + 1);
            co_return move(*retval);
         }

         if (retry >= num_retries)
         {

#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE

            auto html_result = co_await m_browser->coroRuntimeEval(session_id, "document.documentElement.outerHTML");
            SPDLOG_TRACE("document.documentElement.outerHTML: \r\n{}", html_result ? html_result->result.value : html_result.error().formattedMessage());
#endif

            SPDLOG_DEBUG("coroRuntimeEval still returned an error after {} tries, throwing an exception ({})",
                         retry,
                         retval.error().formattedMessage());

            co_return std::unexpected{ Error{ std::move(retval.error()) } };
         }

         SPDLOG_DEBUG("coroRuntimeEval returned an error on try {}, retrying in {}", retry, retry_delay);
         steady_timer timer{ m_browser->getExecutor(), retry_delay };
         co_await timer.async_wait(use_awaitable);
         retry_delay = DurationT(static_cast<DurationT::rep>(retry_delay.count() * backoff_factor));
      }
      std::unreachable();
   }


   awaitable<RuntimeEvalResult> CellarTrackerBrowser::coroEvalWithRetry(std::string session_id, std::string source_js) noexcept(false)
   {
      auto result = co_await coroEvalWithRetry(move(session_id),
                                               move(source_js),
                                               JS_EVAL_RETRY_COUNT,
                                               JS_RETRY_INITIAL_DELAY,
                                               JS_EVAL_RETRY_BACKOFF_FACTOR);

      if (result) co_return move(result.value());

      throw ctb::Error{ move(result.error()) };
   }


   namespace
   {
      constexpr auto LOGGED_IN_STR  = "logged_in:"sv;   // actual return value will be "logged_in:welcome <username>"
      constexpr auto LOGGED_OUT_STR = "logged_out"sv;

      bool loggedIn(const RuntimeEvalResult& result)
      {
         return result.result.value.starts_with(LOGGED_IN_STR);
      };

      std::string getLoginName(const RuntimeEvalResult& result)
      {
         std::string_view result_val = result.result.value;
         return result_val.size() > LOGGED_IN_STR.size() ? std::string{ result_val.substr(LOGGED_IN_STR.size()) } : std::string{};
      };

   }   // namespace


   awaitable<CellarTrackerBrowser::LoginResult> CellarTrackerBrowser::coroCheckLoginStatus(std::string session_id) noexcept
   {
      try
      {
         const auto check_login_status_js = format(params::FMT_CHECK_LOGIN_STATUS_JS, LOGGED_IN_STR, LOGGED_OUT_STR);

         auto eval_result = co_await coroEvalWithRetry(session_id, check_login_status_js);
         if (loggedIn(eval_result))
         {
            co_return LoginStatus{ true, getLoginName(eval_result) };
         }

         SPDLOG_DEBUG("CellarTrackerBrowser::coroCheckLoginStatus received response of type {} with value {}", eval_result.result.type,
                      eval_result.result.value);

         co_return LoginStatus{ false, {} };
      }
      catch (...)
      {
         co_return std::unexpected{ packageError() };
      }
   }


   AnySender<CellarTrackerBrowser::LoginResult> CellarTrackerBrowser::sndAttemptLogin(CredentialWrapper&& cred) noexcept
   {
      if (status() != Status::Ready) return sndNotReadyError(status());

      auto asio_coro =
         [this, cred = move(cred)] mutable -> awaitable<LoginResult>   // NOLINT [cppcoreguidelines-avoid-capturing-lambda-coroutines]
      {
         try
         {
            auto session = co_await m_browser->coroCreateSession();
            if (!session)
            {
               co_return std::unexpected{
                  Error{ Error::Category::NetworkError, "Couldn't get browser session to CT login" }
               };
            }

            // navigate to login page. If we're already logged in it will redirect to main page
            static constexpr auto CT_LOGIN_URL   = "https://www.cellartracker.com/password.asp"sv;
            static constexpr auto CT_DEFAULT_URL = "https://www.cellartracker.com/default.asp"sv;

            auto nav_result = co_await m_browser->coroNavigate(session->sessionId(), std::string{ CT_LOGIN_URL });
            boost::to_lower(nav_result.url);
            auto trimmed_url = trim_back_view(nav_result.url, "/");
            if (trimmed_url != CT_DEFAULT_URL and trimmed_url != CT_LOGIN_URL)
            {
               SPDLOG_DEBUG("CellarTrackerBrowser couldn't navigate to login page, redirected to {}", nav_result.url);
               co_return LoginStatus{ false, "" };
            }

            const auto fill_form_js = format(params::FMT_FILL_LOGIN_FORM_JS, cred.username(), cred.password());

            SPDLOG_DEBUG("Attempting to fill login form fields...");
            auto eval_result = co_await coroEvalWithRetry(session->sessionId(), fill_form_js, JS_EVAL_RETRY_COUNT, 50ms, JS_EVAL_RETRY_BACKOFF_FACTOR);
            if (!eval_result)
            {
               // filling login form will fail if we're already logged in. Try checking login status.
               // have to do this here instead of above catch block because of co_await
               auto login_status = co_await coroCheckLoginStatus(session->sessionId());
               if (login_status.has_value() and login_status->first)
               {
                  // we're logged in.
                  co_return login_status;
               }
               else
               {
                  co_return std::unexpected{ std::move(eval_result.error()) };
               }
            }


            // TODO  check error parsing
            SPDLOG_DEBUG("Attempting to submit login form...");
            eval_result = co_await coroEvalWithRetry(session->sessionId(), std::string{ params::SUBMIT_LOGIN_FORM_EXPRESSION });

            // Now check again and return final result.
            co_return co_await coroCheckLoginStatus(session->sessionId());
         }
         catch (...)
         {
            co_return std::unexpected{ packageError() };
         }
      };

      // execute the lambda coro and return its result as a sender
      return asSender<LoginResult>(m_browser->getExecutor(), move(asio_coro));
   }


   awaitable<std::string> CellarTrackerBrowser::coroGetLabelImageUrl(std::string session_id) noexcept(false)
   {
      static const std::string expression =
         R"((() => { let el = document.getElementById('label_photo') || document.querySelector('.label_photo');
               if (!el) return 'ERROR: element label_photo not found'; if (el.tagName.toLowerCase() === 'img') return el.src;
               if (el.tagName.toLowerCase() === 'a') return el.href; let img = el.querySelector('img');
               if (img) return img.src; return 'ERROR: no image source found in element'; })())";


      auto retval = co_await coroEvalWithRetry(session_id, expression);
      SPDLOG_DEBUG("HeadlessBrowser::coroRuntimeEval returned label URL '{}'", retval.result.value);
      co_return retval.result.value;
   }


   void CellarTrackerBrowser::checkStatus() noexcept(false)
   {
      auto current_status = status();
      if (current_status != Status::Ready)
         throw ctb::Error(Error::Category::GeneralError, constants::FMT_ERROR_HEADLESS_BROWSER_INVALID_STATUS, current_status);
   }


}   // namespace ctb


