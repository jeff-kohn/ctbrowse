#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <exec/asio/use_sender.hpp>
#include <exec/task.hpp>

#include <filesystem>


namespace ctb
{
   // satisfies the scheduler concept for use with a stdexec pipeline via continues_on()
   struct AsioPipelineScheduler
   {
      using scheduler_concept = stdexec::scheduler_t;

      asio::any_io_executor executor{};


      // Named sender type (not auto) so AsioPipelineScheduler::schedule()'s return type
      // isn't circularly dependent on stdexec::scheduler<AsioPipelineScheduler> itself.
      class ScheduleSender
      {
      public:
         using sender_concept    = stdexec::sender_t;
         using scheduler_concept = stdexec::scheduler_t;

         explicit ScheduleSender(asio::any_io_executor exec) noexcept : m_executor(std::move(exec))
         {}

         struct Env
         {
            asio::any_io_executor executor;

            constexpr auto query(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, auto&&...) const noexcept -> AsioPipelineScheduler
            {
               return AsioPipelineScheduler{ executor };
            }
         };

         [[nodiscard]] auto get_env() const noexcept -> Env
         {
            return Env{ m_executor };
         }

         // The concrete raw sender type produced by asio::post(...) + use_sender.
         using RawSender = decltype(asio::post(std::declval<asio::any_io_executor&>(), exec::asio::use_sender));

         // Forward the wrapped sender's actual completion signatures rather than guessing.
         // Must be a non-static member callable as sndr().get_completion_signatures(env...)
         // (the "legacy member" completion-signatures customization point), accepting zero
         // or one environment argument.
         template<typename... Env>
         [[nodiscard]] constexpr auto get_completion_signatures(const Env&...) const noexcept
            -> stdexec::completion_signatures_of_t<RawSender>
         {
            return {};
         }

         template<typename Receiver>
         [[nodiscard]] auto connect(Receiver&& rcvr) const
         {
            return stdexec::connect(asio::post(m_executor, exec::asio::use_sender), std::forward<Receiver>(rcvr));
         }

      private:
         asio::any_io_executor m_executor;
      };


      [[nodiscard]] ScheduleSender schedule() const noexcept
      {
         return ScheduleSender{ executor };
      }

      // for stdexec scheduler concept
      bool operator==(const AsioPipelineScheduler& rhs) const = default;
   };



}
