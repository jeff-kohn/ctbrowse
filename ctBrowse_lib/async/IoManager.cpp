#include "IoManager.h"


namespace ctb
{

   asio::any_io_executor IoManager::get_executor() const
   {
      return m_ctx->get_executor();
   }


   IoManager::ContextPtr IoManager::get_context()
   {
      return m_ctx;
   }


   AsioPipelineScheduler IoManager::get_scheduler() const
   {
      return AsioPipelineScheduler{ get_executor() };
   }


   exec::task<IoManager::ReadFileResult> IoManager::sndReadFile(fs::path file_path) const noexcept
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

         co_await asio::async_read(file, asio::buffer(buf), exec::asio::use_sender);
         co_return buf;
      }
      catch (...)
      {
         auto err     = packageError();
         err.category = Error::Category::FileError;
         co_return std::unexpected{ std::move(err) };
      }
   }

   IoManager::~IoManager()
   {
      try
      {
         m_work_guard.reset();
         if (m_ctx and !m_ctx->stopped()) m_ctx->stop();
      }
      catch (...) // NOLINT
      {
         SPDLOG_DEBUG("IoManager destructor caught an unhandled exception from asio: {}", packageError().formattedMessage());
      }
   }

}   // namespace ctb
