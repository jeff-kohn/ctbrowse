#include "ctb/model/ProReviewsCache.h"

#include <string_view>
#include <sstream>

namespace ctb
{

   namespace
   {
      struct ProSchema
      {
         std::string_view pro_id{};
         std::string_view pro_name{};
         CtProp drink_begin{};
         CtProp drink_end{};
         CtProp score_numeric{};
         CtProp score_text{};
      };

      const std::array ProSchemas =
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
         ostr << ctb::format("{}{}, ", window.pro_id.asStringView(), detail::getDrinkWindow(window.pro_drink_begin, window.pro_drink_end));
      }

      auto result = ostr.str();
      if (result.size() > delim_ct)
      {
         result.erase(result.end() - delim_ct, result.end()); // remove the final ", "
      }
      return result;
   }

#pragma warning(push)
#pragma warning(disable: 4702) // stupid MSVC

   auto ProReviewsCache::getCtDrinkWindow(uint64_t wine_id) const -> std::string
   {

      for (auto window : getDrinkWindows(wine_id))
      {
         return detail::getDrinkWindow(window.ct_drink_begin, window.ct_drink_end);
      }
      return {};

   }

#pragma warning(pop)

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
               m_drink_windows.emplace(wine_id, 
                  ProDrinkWindow{ 
                     .wine_id = wine_id, 
                     .pro_id = std::string{ schema.pro_id }, 
                     .pro_drink_begin = beg, 
                     .pro_drink_end = end, 
                     .ct_drink_begin = rec[CtProp::CtBeginConsume], 
                     .ct_drink_end = rec[CtProp::CtEndConsume]
                  }
               );
            }

            if (auto score = rec[schema.score_text]; score.hasValue())
            {
               m_scores.emplace(wine_id, 
                  ProScore{ 
                     .wine_id = wine_id, 
                     .pro_id = std::string{schema.pro_id},
	   					.score_text = score, 
                     .score_numeric = rec[schema.score_numeric] 
                  }                   
               );
            }
         }
      }
   }

} // namespace ctb