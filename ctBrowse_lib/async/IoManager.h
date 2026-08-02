#pragma once

#include "AsioPipelineScheduler.h"
#include "ctb/ctb.h"

#include <asio/any_io_executor.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <exec/asio/use_sender.hpp>
#include <exec/task.hpp>

#include <expected>
#include <memory>


namespace ctb
{

   /// @brief Class for running ASIO network/IO tasks on a background thread. Uses a jthread to
   ///        run an io_context in the background with a work_guard to keep it alive.
   ///
   /// Can be used as a bridge for different async I/O approaches since it supports:
   ///   a) getting an any_executor
   ///   b) getting a stdexec scheduler
   ///   c) getting a shared_ptr to the io_context itself
   ///
   /// This class also implements some stdexec sender/coroutine methods for file I/O. These senders
   /// are non-throwing and always return an expected<result, ctb::Error> so that error handling approach
   /// can be left to the caller.
   ///
   class IoManager
   {
   public:
      using ContextPtr = std::shared_ptr<asio::io_context>;


      /// @brief get an executor to run tasks on
      [[nodiscard]] asio::any_io_executor get_executor() const;

      // For passing explicitly to stream_file or co_spawn
      [[nodiscard]] ContextPtr get_context();

      // for stdexec pipelines
      AsioPipelineScheduler get_scheduler() const;


      /// @brief The result type for sndReadFile
      using ReadFileResult = std::expected<Buffer, ctb::Error>;

      /// @brief stdexec sender to read a file into a buffer.
      /// @param file_path - path the file to load
      /// @return  sender to retrieve the result asynchronously
      [[nodiscard]] exec::task<ReadFileResult> sndReadFile(fs::path file_path) const noexcept;


      /// @brief return type for sndWriteFile, expected value is bytes written
      using WriteFileResult = std::expected<size_t, Error>;

      /// @brief asynchronously write a file to disk.
      template<rng::range RngT>
      [[nodiscard]] exec::task<WriteFileResult> sndWriteFile(std::string file_path, const RngT& buf) const noexcept
      {
         try
         {
            asio::stream_file file{ get_executor(), file_path,
                                    asio::stream_file::write_only | asio::stream_file::create | asio::stream_file::truncate };
            co_return co_await asio::async_write(file, asio::buffer(buf), exec::asio::use_sender);
         }
         catch (...)
         {
            auto err     = packageError();
            err.category = Error::Category::FileError;
            co_return std::unexpected{ std::move(err) };
         }
      }

      ~IoManager();

      // no copy or move
      IoManager()                            = default;
      IoManager(IoManager&)                  = delete;
      IoManager(IoManager&&)                 = delete;
      IoManager& operator=(const IoManager&) = delete;
      IoManager& operator=(IoManager&&)      = delete;

   private:
      using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

      ContextPtr m_ctx{ std::make_shared<ContextPtr::element_type>(1) };
      WorkGuard  m_work_guard{ asio::make_work_guard(*m_ctx) };
      // clang-format off
      std::jthread m_thread{ [this]() { m_ctx->run(); } };
      // clang-format on
   };

}   // namespace ctb
