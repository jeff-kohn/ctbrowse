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

   inline auto sndNotReadyError(CellarTrackerBrowser::Status status)
   {
      return stdexec::just_error(std::make_exception_ptr(
         ctb::Error{ Error::Category::GeneralError, constants::FMT_ERROR_HEADLESS_BROWSER_INVALID_STATUS, status }));
   }

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


   AnySender<HttpFileContents> CellarTrackerBrowser::sndDownloadLabel(uint64_t wine_id) noexcept(false)
   {
      if (status() != Status::Ready) return sndNotReadyError(status());

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
      return asSender<HttpFileContents>(m_browser->getExecutor(), asio_coro);
   }


   template<DurationType DurationT>
   asio::awaitable<RuntimeEvalResult> CellarTrackerBrowser::coroEvalWithRetry(std::string session_id,
                                                                              std::string source_js,
                                                                              uint16_t    num_retries,
                                                                              DurationT   retry_delay,
                                                                              double      backoff_factor) noexcept(false)
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
            SPDLOG_DEBUG("coroRuntimeEval still returned an error after {} tries, throwing an exception ({})",
                         retry,
                         retval.error().formattedMessage());
            throw move(retval.error());
         }

         SPDLOG_DEBUG("coroRuntimeEval returned an error on try {}, retrying in {}", retry, retry_delay);
         asio::steady_timer timer{ m_browser->getExecutor(), retry_delay };
         co_await timer.async_wait(use_awaitable);
         retry_delay = DurationT(static_cast<typename DurationT::rep>(retry_delay.count() * backoff_factor));
      }
      std::unreachable();
   }


   asio::awaitable<RuntimeEvalResult> CellarTrackerBrowser::coroEvalWithRetry(std::string session_id, std::string source_js) noexcept(false)
   {
      co_return co_await coroEvalWithRetry(move(session_id),
                                           move(source_js),
                                           JS_EVAL_RETRY_COUNT,
                                           JS_RETRY_INITIAL_DELAY,
                                           JS_EVAL_RETRY_BACKOFF_FACTOR);
   }


   namespace
   {
      static constexpr auto LOGGED_IN_STR  = "logged_in:"sv;   // actual return value will be "logged_in:username"
      static constexpr auto LOGGED_OUT_STR = "logged_out"sv;
      static constexpr auto UNKNOWN_STR    = "unknown"sv;

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


   asio::awaitable<CellarTrackerBrowser::LoginStatus> CellarTrackerBrowser::coroCheckLoginStatus(const std::string& session_id)
   {
      const auto check_login_status_js = format(params::FMT_CHECK_LOGIN_STATUS_JS, LOGGED_IN_STR, LOGGED_OUT_STR, UNKNOWN_STR);

      auto eval_result = co_await coroEvalWithRetry(session_id, check_login_status_js);
      if (loggedIn(eval_result))
      {
         co_return LoginStatus{ true, getLoginName(eval_result) };
      }

      SPDLOG_DEBUG("CellarTrackerBrowser::coroCheckLoginStatus received response of type {} with value {}", eval_result.result.type,
                   eval_result.result.value);

      co_return LoginStatus{ false, {} };
   }


   AnySender<CellarTrackerBrowser::LoginResult> CellarTrackerBrowser::sndAttemptLogin(CredentialWrapper&& cred) noexcept
   {
      if (status() != Status::Ready) return sndNotReadyError(status());

      auto asio_coro = [this, cred = move(cred)] mutable -> asio::awaitable<LoginResult>
      {
         auto session = co_await m_browser->coroCreateSession();
         if (!session)
         {
            throw ctb::Error{ Error::Category::NetworkError, "Couldn't get browser session to CT login" };
         }

         // navigate to login page. If we're already logged in it will redirect to main page, if not we can fill
         // form and submit.
         static constexpr auto DEFAULT_PAGE = "default.asp"sv;
         static constexpr auto LOGIN_PAGE   = "password.asp"sv;
         static constexpr auto CT_LOGIN_URL = "https://www.cellartracker.com/password.asp"sv;

         auto nav_result   = co_await m_browser->coroNavigate(session->sessionId(), std::string{ CT_LOGIN_URL });
         boost::to_lower(nav_result.url);
         auto trimmed_url = trim_back_view(nav_result.url, "/");
         if (trimmed_url.ends_with(DEFAULT_PAGE))
         {
            // If we get redirected to default.asp it should mean we're already logged in.
            co_return co_await coroCheckLoginStatus(session->sessionId());
         }

         if (!trimmed_url.ends_with(LOGIN_PAGE))
         {
            // If we're on any other page than password.asp, bail the fuck out
            SPDLOG_DEBUG("CellarTrackerBrowser couldn't navigate to login page, redirected to {}", nav_result.url);
            co_return LoginStatus{ false, "" };
         }

         // OK so we need to login. Start by filling the form fields and clicking submit
         // We disable retries since we're interacting with previously loaded page, but still use coroEvelWithRetry()
         // because it unwraps the expected<> and throws on error for us, which is preferred here.
         const auto fill_login_form_js = format(params::FMT_FILL_LOGIN_FORM_JS, cred.username(), cred.password());
         auto       eval_result        = co_await coroEvalWithRetry(session->sessionId(), fill_login_form_js, 0, 0ms, 0);

         // TODO  check error parsing
         eval_result = co_await coroEvalWithRetry(session->sessionId(), std::string{ params::SUBMIT_LOGIN_FORM_EXPRESSION }, 0, 0ms, 0);

         // Now check again and return final result.
         co_return co_await coroCheckLoginStatus(session->sessionId());
      };

      // execute the lambda coro and return its result as a sender
      return asSender<LoginResult>(m_browser->getExecutor(), move(asio_coro));
   }


   asio::awaitable<std::string> CellarTrackerBrowser::coroGetLabelImageUrl(std::string session_id) noexcept(false)
   {
      static const std::string expression =
         R"((() => { let el = document.getElementById('label_photo') || document.querySelector('.label_photo');
               if (!el) return 'ERROR: element label_photo not found'; if (el.tagName.toLowerCase() === 'img') return el.src;
               if (el.tagName.toLowerCase() === 'a') return el.href; let img = el.querySelector('img');
               if (img) return img.src; return 'ERROR: no image source found in element'; })())";

      constexpr auto RETRY_COUNT    = 4u;
      auto           retry_interval = 500ms;

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
      std::unreachable();
   }


   void CellarTrackerBrowser::checkStatus() noexcept(false)
   {
      auto current_status = status();
      if (current_status != Status::Ready)
         throw ctb::Error(Error::Category::GeneralError, constants::FMT_ERROR_HEADLESS_BROWSER_INVALID_STATUS, current_status);
   }


}   // namespace ctb


