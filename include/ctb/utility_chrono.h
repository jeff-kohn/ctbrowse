/*******************************************************************
* @file utility_chrono.h
*
* @brief Header file for some chrono-related helper functions
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/#pragma once

#include "ctb/ctb.h"

#include <fmt/chrono.h>
#include <chrono>
#include <expected>
#include <spanstream>
#include <string_view>
#include <string>


namespace ctb
{
   using namespace std::literals::chrono_literals;
   namespace chrono = std::chrono;


   /// @brief Parse an ISO date-time string and return it as a UTC timepoint. 
   /// 
   /// Values returned from this function will always be UTC, even if the string
   /// contained a timezone offeset. 
   /// 
   /// @return expected value parsed date, unexpected value is exception object containing error information
   /// 
   [[nodiscard]] inline auto parseIsoDateTime(std::string_view dt_str) -> std::expected<chrono::sys_seconds, ctb::Error>
   {
      // first try parsing with time zone offset, if that fails try parsing as UTC.
      std::ispanstream dt_strm{ dt_str };
      chrono::sys_seconds ts{};

      dt_strm >> parse(constants::PARSE_FMT_STR_ISO_DATETIME_LOCAL, ts);
      if (dt_strm.fail())
      {
         dt_strm = std::ispanstream{ dt_str };
         dt_strm >> parse(constants::PARSE_FMT_STR_ISO_DATETIME_UTC, ts);
      }
      if (dt_strm.fail())
         return std::unexpected{ Error{ Error::Category::ParseError, "The intput string '{}' could not be parsed as a valid date/time", dt_str } };
      else return ts;
   }


   /// @brief Parse an ISO date string into a year_month_day
   /// 
   [[nodiscard]] inline auto parseIsoDate(std::string_view dt_str) -> std::expected<chrono::year_month_day, Error> 
   {
      constexpr auto format_str{ constants::PARSE_FMT_STR_ISO_DATE_ONLY };

      std::ispanstream dt_strm{ dt_str, };
      chrono::sys_days days{};
      if ((dt_strm >> parse(format_str, days)))
         return chrono::year_month_day(days);
      else
         return std::unexpected{ Error{ Error::Category::ParseError, "The intput string '{}' could not be parsed as a valid date", dt_str } };
   }


   /// @brief Format a date/time as ISO string that can be used with REST calls.
   ///
   /// This function should work for time_point's and month_day_year's, as well as any other
   /// value that format() knows how to format as a date/time.
   /// 
   template<typename DateType>
   [[nodiscard]] inline std::string toIsoDateTime(DateType&& date_val)
   {
      return format("{:%FT%TZ}", chrono::floor<chrono::seconds>(std::forward<DateType>(date_val)) );
   }


   /// @brief  Convert a year_month_day to a ISO date string
   /// 
   [[nodiscard]] inline std::string toIsoDate(const chrono::year_month_day& date)
   {
      return format("{:%F}", date);
   }


} // namespace ctb