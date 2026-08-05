#pragma once

#include "webclient_schema.h"

#include <asio/experimental/channel.hpp>
#include <asio/use_awaitable.hpp>

#include <map>
#include <string_view>
#include <unordered_set>


namespace ctb
{

   class BrowserEvents
   {
   public:
      static constexpr auto CHANNEL_SIZE = 10;

      BrowserEvents(const asio::any_io_executor& ex, std::string session_id)
         : m_channel{ std::in_place, ex, CHANNEL_SIZE },
           m_session_id{ std::move(session_id) }
      {}

      template<rng::input_range RngT> requires std::constructible_from<std::string, rng::range_value_t<RngT>>
      BrowserEvents(const asio::any_io_executor& ex, std::string session_id, RngT&& event_names)
         : m_channel{ std::in_place, ex, CHANNEL_SIZE },
           m_session_id{ std::move(session_id) },
           m_subscribed_events{ std::from_range, std::forward<RngT>(event_names) }
      {}

      const std::string& sessionId() const
      {
         return m_session_id;
      }


      void subscribeEvent(std::string event_name)
      {
         m_subscribed_events.emplace(std::move(event_name));
      }

      void unSubscribeEvent(const std::string& event_name)
      {
         m_subscribed_events.erase(event_name);
      }

      template<rng::input_range RngT> requires std::constructible_from<std::string, rng::range_value_t<RngT>>
      void subscribeEvents(RngT&& event_names)
      {
         m_subscribed_events.insert_range(std::forward<RngT>(event_names));
      }


      template<rng::input_range RngT> requires std::constructible_from<std::string, rng::range_value_t<RngT>>
      void unSubscribeEvents(const RngT& event_names)
      {
         rng::for_each(event_names,
                       [this](auto event_name)
                       {
                          unSubscribeEvent(event_name);
                       });
      }

      void unSubscribeAllEvents()
      {
         m_subscribed_events.clear();
      }


      /// @brief used by client to get/wait for next subscribed event.
      asio::awaitable<BrowserMessage> coroAwaitEvent()
      {
         co_return co_await m_channel->async_receive(asio::use_awaitable);
      }

      /// @brief used by HeadlessBrowser to post events as they come in. You can
      ///        pass it whatever, but only subscribed events will actually get posted
      ///        to the channel.
      void postEvent(BrowserMessage msg)
      {
         const auto& maybe_method = msg.method;
         if (maybe_method and m_subscribed_events.contains(*maybe_method))
         {
            if (!m_channel->try_send(std::error_code{}, std::move(msg)))
            {
               SPDLOG_DEBUG("Warning: couldn't push BrowserMessage for event {} to channel", *maybe_method);
               assert(false);
            }
         }
      }

      BrowserEvents(BrowserEvents&&)            = default;
      BrowserEvents& operator=(BrowserEvents&&) = default;
      ~BrowserEvents() noexcept                 = default;

      BrowserEvents()                                = delete;
      BrowserEvents(const BrowserEvents&)            = delete;
      BrowserEvents& operator=(const BrowserEvents&) = delete;

   private:
      using EventChannel     = asio::experimental::channel<void(std::error_code, BrowserMessage)>;
      using UnorderedStrings = std::unordered_set<std::string>;

      indirect<EventChannel> m_channel;
      std::string            m_session_id{};
      UnorderedStrings       m_subscribed_events{};
   };


   using BrowserEventsPtr = std::shared_ptr<BrowserEvents>;


}   // namespace ctb
