#pragma once

#include "ctb/ctb.h"
#include "ctb/tables/ProReviewsCacheTraits.h"

#include <map>
#include <string>

namespace ctb
{
   struct ProScore
   {
      uint64_t      wine_id{};
      CtPropertyVal pro_id{};         // 2-3 letter code used for abbreviated scores e.g. JD90
      CtPropertyVal score_text{};
      CtPropertyVal score_numeric{};
   };

   struct ProDrinkWindow
   {
      uint64_t      wine_id{};
      CtPropertyVal pro_id{};         
      CtPropertyVal pro_drink_begin{};
      CtPropertyVal pro_drink_end{};
      CtPropertyVal ct_drink_begin{};
      CtPropertyVal ct_drink_end{};
   };

   class ProReviewsCache
   {
   public:
      
      ProReviewsCache(const ProReviewsCacheTable& tbl)
      {
         processDataset(tbl);
      }

      auto getScores(uint64_t wine_id) const
      {
         auto [first, last] = m_scores.equal_range(wine_id);
         return std::ranges::subrange{ first, last } | vws::values;
      }

      auto getScoreSummary(uint64_t wine_id) const -> std::string;

      auto getDrinkWindows(uint64_t wine_id) const
      {
         auto [first, last] = m_drink_windows.equal_range(wine_id);
         return std::ranges::subrange{ first, last } | vws::values;
      }

      auto getDrinkWindowSummary(uint64_t wine_id) const -> std::string;

      void swap(ProReviewsCache& other) noexcept
      {
         using std::swap;
         swap(m_scores,        other.m_scores);
         swap(m_drink_windows, other.m_drink_windows);
      }

      auto getCtDrinkWindow(uint64_t wine_id) const -> std::string;

   private:
      std::multimap<uint64_t, ProScore>       m_scores{};
      std::multimap<uint64_t, ProDrinkWindow> m_drink_windows{};

      void processDataset(const ProReviewsCacheTable& tbl);
   };

   inline void swap(ProReviewsCache& lhs, ProReviewsCache& rhs) noexcept
   {
      lhs.swap(rhs);
   }

} // namespace ctb