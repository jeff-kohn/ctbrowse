#pragma once

#include <fmt/format.h>

namespace ctb
{
   // All project that need format() related functions should use them through this header
   // this allows us to easily switch between std:: and fmt:: if we want.
   //
   using fmt::format;
   using fmt::format_string;
   using fmt::make_format_args;
   using fmt::vformat;

} // namespace ctb