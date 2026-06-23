#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/io_context.hpp>
#include <asio/executor_work_guard.hpp>


namespace ctb
{
   /// @brief Class for running ASIO network/IO tasks on a background thread. Uses a jthread to
   ///        run an io_context in the background with a work_guard to keep it alive. When destructor
   ///        is called the work_guard will be reset and the io_executor will wait for any pending tasks
   ///        to finish allowing for a clean shutdown.,
   class BackgroundThreadContext
   {
   public:
      /// @brief get an executor to run tasks on
      [[nodiscard]] auto get_executor() -> asio::any_io_executor
      {
         return m_ctx.get_executor();
      }

      // For passing explicitly to stream_file or co_spawn
      [[nodiscard]] auto get_context() -> asio::io_context&
      {
         return m_ctx;
      }

      /// @brief destructor
      ~BackgroundThreadContext()
      {
         try
         {
            // stop the busy work, context will finish any pending task and then shut down.
            m_work_guard.reset();
         }
         catch (...) // NOLINT
         {}   
      }

      // no copy or move
      BackgroundThreadContext()                                          = default;
      BackgroundThreadContext(BackgroundThreadContext&)                  = delete;
      BackgroundThreadContext(BackgroundThreadContext&&)                 = delete;
      BackgroundThreadContext& operator=(const BackgroundThreadContext&) = delete;
      BackgroundThreadContext& operator=(BackgroundThreadContext&&)      = delete;

   private:
      using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

      asio::io_context m_ctx{};
      WorkGuard        m_work_guard{ asio::make_work_guard(m_ctx) };
      std::jthread     m_thread{ [this](){ m_ctx.run(); } };
   };

}   // namespace ctb
