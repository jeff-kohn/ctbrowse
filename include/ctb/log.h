/*********************************************************************
 * @file       log.h
 *
 * @brief      logging interface for the app. 
 *             namespace
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "ctb/Error.h"

// default active level is Info, which means in release builds all
// logging done with SPDLOG_DEBUG() will be omitted at compile time but 
// included for debug builds.
#if !defined(NDEBUG)
   #define SPDLOG_ACTIVE_LEVEL 1 // SPDLOG_LEVEL_DEBUG
#endif

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <memory>
#include <filesystem>
#include <string>
#include <string_view>
#include <source_location>


/// @brief Logging namespace for ctBrowse. 
/// 
/// This ctBrowse_lib library logs debug-level messages only in Debug builds. Release builds do not generate any log output.
/// 
/// The ctBrowse_lib library uses the default spdlog sink, which just outputs messages to stdout/stderr, unless an application
/// calls setupDefaultLogger() to configure other log outputs.
///        
namespace ctb::log
{

   // import these symbols into our namespace, so logging interface looks like log::warn(...) and there's
   // possibility to replace logging backend with minimal impact if needed in the future.
   //
   namespace fs = std::filesystem;
   namespace sinks = spdlog::sinks;
   namespace log_level = spdlog::level;
   using spdlog::logger;
   using spdlog::source_loc;
   using spdlog::level::level_enum;
   using spdlog::sinks::rotating_file_sink_mt;
   using spdlog::sinks_init_list;
   using spdlog::set_default_logger;
   using spdlog::shutdown;
   using spdlog::log;
   using spdlog::error;
   using spdlog::warn;
   using spdlog::info;
   using spdlog::trace;
   using spdlog::critical;
   using log_ptr_t = std::shared_ptr<logger>;
   using sink_ptr_t = sinks_init_list::value_type;

#if defined(_WIN32)
   using spdlog::sinks::msvc_sink_mt;
#endif

} // namespace ctb::log


namespace ctb::constants
{
   inline constexpr const char* LOG_NAME = "ctb";
   inline constexpr const char* LOG_PATTERN_CONSOLE  = "[%^%l%$] %v";
   inline constexpr const char* LOG_PATTERN_DEBUGGER = "[%n Thread %t][%^%l%$] %v";
   inline constexpr const char* LOG_PATTERN_FILE     = "[%Y-%m-%d %H:%M:%S.%e][TID %t][%^%l%$] %v";

#if !defined(NDEBUG)
   inline constexpr auto LOGLEVEL_GLOBAL   = log::level_enum::debug;
   inline constexpr auto LOGLEVEL_FILE     = log::level_enum::debug;
   inline constexpr auto LOGLEVEL_CONSOLE  = log::level_enum::info;
   inline constexpr auto LOGLEVEL_DEBUGGER = log::level_enum::info;
#else
   inline constexpr auto LOGLEVEL_GLOBAL   = log::level_enum::info;
   inline constexpr auto LOGLEVEL_FILE     = log::level_enum::warn;
   inline constexpr auto LOGLEVEL_CONSOLE  = log::level_enum::warn;
   inline constexpr auto LOGLEVEL_DEBUGGER = log::level_enum::off;
#endif

} // namespace ctb::constants


namespace ctb::log
{
   /// @brief Log an exception with source information. 
   /// 
   void exception(const std::exception& e, std::source_location source_loc = std::source_location::current());


   /// @brief Log an exception with source information. 
   /// 
   void exception(const ctb::Error& e, std::source_location source_loc = std::source_location::current());


   /// @brief flush the active default logger's queue to disk
   ///
   inline void flush()
   {
      spdlog::default_logger()->flush();
   }


   /// @brief create a color stdout logging sink
   /// @return the requested sink. may throw on error
   /// 
   [[nodiscard]] auto makeConsoleSink(level_enum level, std::string_view pattern = constants::LOG_PATTERN_CONSOLE) -> sink_ptr_t;


   /// @brief create a logging sink that outputs to OutputDebugString()
   /// @return the requested sink if enabled (debug windows), or null_sink if not
   /// 
   [[nodiscard]] auto makeDebuggerSink() -> sinks_init_list::value_type;


   /// @brief create a sink that logs to file
   /// 
   [[nodiscard]] auto makeFileSink(fs::path log_folder, std::string_view log_filename_base, 
                                   std::string_view pattern = constants::LOG_PATTERN_FILE,
                                   level_enum level = constants::LOGLEVEL_FILE) -> sinks_init_list::value_type;


   /// @brief create and set the default logger so that free-standing log functions will use it.
   /// @param sinks the logging sinks to use with the logger (defaults to debugger and file sinks)
   /// 
   /// while a pointer to the created logger is returned, you don't need to use or hold it since
   /// spdlog has its own internal shared_ptr to the default logger.
   /// 
   auto setupDefaultLogger(sinks_init_list sinks) -> log_ptr_t;


} // namespace ctb::log


