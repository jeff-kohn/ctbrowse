/*********************************************************************
 * @file       log.cpp
 *
 * @brief      logging-related function implementations for the app. 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "ctb/log.h"
#include "ctb/utility.h"

#include <spdlog/sinks/null_sink.h>


namespace ctb::log
{
   /// @brief Log an exception with source information. 
   /// 
   void exception(const std::exception& e, std::source_location source_loc)
   {
      std::string path{ source_loc.file_name() };
      auto file_name = viewFilename(path);
      log::error("{} (in {}:{}) - {}", file_name, source_loc.line(), source_loc.function_name(), e.what());
   }


   /// @brief Log an exception with source information. 
   /// 
   void exception(const ctb::Error& e, std::source_location source_loc)
   {
      std::string path{ source_loc.file_name() };
      auto file_name = viewFilename(path);
      log::error("{} (in {}:{}) - {}", file_name, source_loc.line(), source_loc.function_name(), e.formattedMesage());
   }


   [[nodiscard]] sink_ptr_t makeConsoleSink(level_enum level, std::string_view pattern)
   {
      auto sink = std::make_shared<sinks::stdout_color_sink_mt>();
      sink->set_level(level);
      sink->set_pattern(std::string{ pattern });
      return sink;
   }


   [[nodiscard]] sinks_init_list::value_type makeDebuggerSink()
   {
#if !defined(NDEBUG) && defined(_WIN32)
      // log output to MSVC debug window.
      auto windbg_sink = std::make_shared<msvc_sink_mt>();
      windbg_sink->set_level(constants::LOGLEVEL_DEBUGGER);
      return windbg_sink;
#else
      return std::make_shared<spdlog::sinks::null_sink_mt>();
#endif
   }


   [[nodiscard]] sinks_init_list::value_type makeFileSink(
      fs::path log_folder, 
      std::string_view log_filename_base, 
      std::string_view pattern, 
      level_enum level)
   {
      constexpr auto MAX_FILE_SIZE = constants::ONE_MB * 100;
      constexpr auto MAX_FILE_COUNT = 4;

      auto log_path{ log_folder / log_filename_base };
      if (!log_path.has_extension())
         log_path.replace_extension(".log");

      std::string logfile{ log_path.generic_string() };
      tryExpandEnvironmentVars(logfile);

      auto file_sink = make_shared<rotating_file_sink_mt>(logfile, MAX_FILE_SIZE, MAX_FILE_COUNT);
      file_sink->set_level(level);
      file_sink->set_pattern(std::string{ pattern });
      return file_sink;

   }


   log_ptr_t setupDefaultLogger(sinks_init_list sinks)
   {
      constexpr auto QUEUE_SIZE = 8192;
      constexpr auto THREAD_POOL_SIZE = 1;
      spdlog::init_thread_pool(QUEUE_SIZE, THREAD_POOL_SIZE);
      auto logger = std::make_shared<spdlog::async_logger>(constants::LOG_NAME, sinks, spdlog::thread_pool());
      logger->set_level(constants::LOGLEVEL_GLOBAL);

      // name collision with existing logger will throw.
      spdlog::drop(logger->name()); 
      set_default_logger(logger);
      return logger;
   }



} // namespace ctb::log