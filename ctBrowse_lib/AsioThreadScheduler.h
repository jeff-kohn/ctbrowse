#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <exec/asio/use_sender.hpp>

#include <memory>

namespace ctb
{
   /// @brief Class for running ASIO network/IO tasks on a background thread. Uses a jthread to
   ///        run an io_context in the background with a work_guard to keep it alive. When destructor
   ///        is called the work_guard will be reset and the io_executor will wait for any pending tasks
   ///        to finish allowing for a clean shutdown.
   ///
   /// Can be used as a bridge for different async I/O approaches since it supports:
   ///   a) getting an any_executor
   ///   b) getting a stdexec scheduler
   ///   c) getting a shared_ptr to the io_context itself

   class AsioThreadScheduler
   {
   public:
      using ContextPtr     = std::shared_ptr<asio::io_context>;
      using ReadFileResult = std::expected<Buffer, ctb::Error>;


      // satisfies the scheduler concept for use with a stdexec pipeline via continues_on()
      struct PipelineScheduler
      {
         asio::any_io_executor executor{};

         auto schedule() const
         {
            return asio::post(executor, exec::asio::use_sender);
         }

         // for stdexec scheduler concept
         bool operator==(const PipelineScheduler& rhs) const = default;
      };

      /// @brief get an executor to run tasks on
      [[nodiscard]] asio::any_io_executor get_executor() const
      {
         return m_ctx->get_executor();
      }

      // For passing explicitly to stream_file or co_spawn
      [[nodiscard]] ContextPtr get_context()
      {
         return m_ctx;
      }

      // for stdexec pipelines
      PipelineScheduler get_scheduler() const
      {
         return PipelineScheduler{ get_executor() };
      }

      asio::awaitable<ReadFileResult> coroReadFile(const fs::path& file_path) const
      {
         try
         {
            if (!fs::exists(file_path))
            {
               co_return std::unexpected{
                  Error{ ERROR_FILE_NOT_FOUND, Error::Category::FileError, "File \"{}\" not found.", file_path.generic_string() }
               };
            }

            asio::stream_file file{ get_executor(), file_path.generic_string(), asio::stream_file::read_only };

            auto   file_size = file.size();
            Buffer buf(file_size);

            co_await asio::async_read(file, asio::buffer(buf), asio::use_awaitable);
            co_return buf;
         }
         catch (...)
         {
            auto err     = packageError();
            err.category = Error::Category::FileError;
            co_return std::unexpected{ std::move(err) };
         }
      }




      ~AsioThreadScheduler()
      {
         m_work_guard.reset();
         if (m_ctx and !m_ctx->stopped()) m_ctx->stop();
      }

      // no copy or move
      AsioThreadScheduler()                                      = default;
      AsioThreadScheduler(AsioThreadScheduler&)                  = delete;
      AsioThreadScheduler(AsioThreadScheduler&&)                 = delete;
      AsioThreadScheduler& operator=(const AsioThreadScheduler&) = delete;
      AsioThreadScheduler& operator=(AsioThreadScheduler&&)      = delete;

   private:
      using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

      ContextPtr m_ctx{ std::make_shared<ContextPtr::element_type>(1) };
      WorkGuard  m_work_guard{ asio::make_work_guard(*m_ctx) };
      // clang-format off
      std::jthread m_thread{ [this]() { m_ctx->run(); } };
      // clang-format on
   };

}   // namespace ctb
