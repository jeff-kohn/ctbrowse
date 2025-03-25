/*********************************************************************
 * @file       log.cpp
 *
 * @brief      logging-related function implementations for the app. 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#include "app_constants.h"
#include "log.h"
#include <spdlog/sinks/null_sink.h>


namespace ctb::log
{

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


   [[nodiscard]] sinks_init_list::value_type makeFileSink(level_enum level, fs::path log_folder, std::string_view log_filename_base, std::string_view pattern)
   {
      constexpr auto MAX_FILE_SIZE = constants::ONE_MB * 100;
      constexpr auto MAX_FILE_COUNT = 4;

      auto log_path{ log_folder / log_filename_base };
      if (!log_path.has_extension())
         log_path.replace_extension(".log");

      std::string logfile{ log_path.generic_string() };
      util::tryExpandEnvironmentVars(logfile);

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