#pragma once

#include <concepts>
#include <ranges>
#include <string_view>

namespace cts
{
   namespace rng = std::ranges;
   namespace vws = rng::views;

   /// @brief concept for a type that is convertible to std::string_view
   template <typename T>
   concept StringViewCompatible = std::convertible_to<T, std::string_view>;


} // namespace cts
