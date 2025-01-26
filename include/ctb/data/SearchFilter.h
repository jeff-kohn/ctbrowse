#pragma once

#include "ctb/ctb.h"

#include <string_view>
#include <vector>


namespace ctb
{
   
   template <TableEntry T>
   struct SearchFilter
   {
   public:
      using Prop = T::Prop;
      using ValueWrapper = T::ValueWrapper;
      using PropsList = std::vector<Prop>;

      template <rng::InputRange Rng> requires std::is_same_v<rng::range_value_t<Rng>, Prop>
      void setSearchProps(Rng&& props)
      { 
         PropsList props{ std::forward<Rng>(props) };
         m_search_props.swap(props);
      }

      bool operator()(T& rec) const
      {


         for (auto prop : m_search_props)
         {
            auto val = 
         }
      }

      std::string search_value{};
      std::vector<Prop> m_search_props{};
      
   }


} // namespace ctb