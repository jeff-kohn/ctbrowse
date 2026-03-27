#include "ctb/model/ProReviewsCache.h"

#include <sstream>

namespace ctb
{

   namespace
   {
      struct ProSchema
      {
         std::string pro_id{};
         std::string pro_name{};
         CtProp drink_begin{};
         CtProp drink_end{};
         CtProp score_numeric{};
         CtProp score_text{};
      };

      static const std::array ProSchemas =
      {
         ProSchema{.pro_id = "DR",  .pro_name = "Decanter",           .drink_begin = CtProp::DR_DrinkBegin,  .drink_end = CtProp::DR_DrinkEnd,  .score_numeric = CtProp::DR_ScoreNumeric,  .score_text = CtProp::DR_ScoreDisplay  },
         ProSchema{.pro_id = "JD",  .pro_name = "Jeb Dunnuck",        .drink_begin = CtProp::NoField,        .drink_end = CtProp::NoField,      .score_numeric = CtProp::JD_ScoreNumeric,  .score_text = CtProp::JD_ScoreDisplay  },
         ProSchema{.pro_id = "JR",  .pro_name = "Jancis Robinson",    .drink_begin = CtProp::JR_DrinkBegin,  .drink_end = CtProp::JR_DrinkEnd,  .score_numeric = CtProp::JR_ScoreNumeric,  .score_text = CtProp::JR_ScoreDisplay  },
         ProSchema{.pro_id = "WFW", .pro_name = "World of Fine Wine", .drink_begin = CtProp::NoField,        .drink_end = CtProp::NoField,      .score_numeric = CtProp::WFW_ScoreNumeric, .score_text = CtProp::WFW_ScoreDisplay },
      };

   } // namespace


   auto ProReviewsCache::getScoreSummary(uint64_t wine_id) const -> std::string
   {
      static constexpr auto delim_ct = 2;
      std::ostringstream ostr{}; 

      auto scores = getScores(wine_id);
      for (const auto& score : scores)
      {
         ostr << ctb::format("{}{}, ", score.pro_id.asStringView(), score.score_text.asStringView());
      }

      auto result = ostr.str();
      if (result.size() > delim_ct)
      {
         result.erase(result.end() - delim_ct, result.end()); // remove the final ", "
      }
      return result;
   }


   auto ProReviewsCache::getDrinkWindowSummary(uint64_t wine_id) const -> std::string
   {
      static constexpr auto delim_ct = 2;
      std::ostringstream ostr{};

      auto windows = getDrinkWindows(wine_id);
      for (const auto& window : windows)
      {
         ostr << ctb::format("{}{}, ", window.pro_id.asStringView(), detail::getDrinkWindow(window.drink_begin, window.drink_end));
      }

      auto result = ostr.str();
      if (result.size() > delim_ct)
      {
         result.erase(result.end() - delim_ct, result.end()); // remove the final ", "
      }
      return result;
   }


   void ProReviewsCache::processDataset(const ProReviewsCacheTable& tbl)
   {
      for (auto&& rec : tbl)
      {
         auto wine_id = rec[CtProp::iWineId].asUInt64().value_or(0);
         assert(wine_id != 0);

         for (auto&& schema : ProSchemas)
         {
            if (auto beg = rec[schema.drink_begin], end = rec[schema.drink_end]; beg.hasValue() or end.hasValue())
            {
               m_drink_windows.emplace(std::make_pair(wine_id, ProDrinkWindow{ .wine_id = wine_id, .pro_id = schema.pro_id, .drink_begin = beg, .drink_end = end }));
            }

            if (auto score = rec[schema.score_text]; score.hasValue())
            {
               m_scores.emplace(std::make_pair(wine_id, ProScore{ .wine_id = wine_id, .pro_id = schema.pro_id, .score_text = score, .score_numeric = rec[schema.score_numeric] }));
            }
         }
      }
   }

} // namespace ctb