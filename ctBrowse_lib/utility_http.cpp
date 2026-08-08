#include "ctb/utility_http.h"
#include "external/HttpStatusCodes.h"

#include <boost/algorithm/string.hpp>


namespace ctb
{
   auto percentEncode(std::string_view text) noexcept -> std::string
   {
      static constexpr auto unreserved = [](char c)
      {
         return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
      };

      std::string result;
      result.reserve(text.size());
      for (auto c : text)
      {
         if (unreserved(c))
            result += c;
         else
            std::format_to(std::back_inserter(result), "%{:02X}", c);
      }
      return result;
   }

   auto percentDecode(std::string_view text) noexcept -> std::string
   {
      std::string result;
      result.reserve(text.size());
      for (size_t i = 0; i < text.size(); ++i)
      {
         if (text[i] == '%' && i + 2 < text.size())
         {
            auto         hex = text.substr(i + 1, 2);
            unsigned int value{};
            auto [ptr, ec] = std::from_chars(hex.data(), hex.data() + hex.size(), value, 16);
            if (ec == std::errc{})
            {
               result += static_cast<char>(value);
               i      += 2;
               continue;
            }
         }
         result += text[i];
      }
      return result;
   }


   //auto parseLabelUrlFromHtml(const std::string& html) -> std::string
   //{
   //   try
   //   {
   //      // parse the HTML to get the URL for the label image.
   //      HtmlParser::Parser parser;
   //      HtmlParser::DOM    dom = parser.Parse(html);
   //      HtmlParser::Query  query(dom.Root());
   //      auto               images = dom.GetElementById(constants::HTML_ELEM_LABEL_PHOTO);
   //      if (images and !images->Children.empty())
   //      {
   //         return images->Children[0]->GetAttribute(constants::HTML_ATTR_SRC);
   //      }
   //   }
   //   catch ([[maybe_unused]] std::exception& e)
   //   {
   //      SPDLOG_DEBUG("parseLabelUrlFromHtml returning empty string due to exception {}", e.what());
   //   }
   //   return {};
   //}


   auto getTextEncodingFromHeader(std::string content_type_header) -> std::optional<TextEncoding>
   {
      static constexpr auto CHARSET_KEY = "charset="sv;

      // sanity check
      if (content_type_header.length() < CHARSET_KEY.length()) return {};

      boost::to_lower(content_type_header);
      auto params = content_type_header | std::views::split(';');
      for (const auto& substr : params)
      {
         auto param = trim_view(std::string_view{ substr.data(), substr.size() });
         if (param.starts_with(CHARSET_KEY))
         {
            if (auto loc = param.find('='); loc < param.size())
            {
               return charsetToCodepage(param.substr(++loc));
            }
         }
      }
      return {};
   }

}   // namespace ctb
