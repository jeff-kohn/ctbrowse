#pragma once

#include "ctb/ctb.h"

#include <boost/algorithm/string.hpp>
#include <expected>
#include <format>
#include <string_view>
#include <vector>


namespace ctb::data
{
   namespace detail
   {

   }
   

   /// @brief implements a substring-matching filter for a table entry/record
   template <TableEntry T>
   struct SubStringFilter
   {
   public:
      using Prop = T::Prop;

      /// @brief the substring to search for.
      std::string search_value{};

      /// @brief the properties to search for.
      std::vector<Prop> search_props{};


      /// @brief  function operator used to perform the substring search. 
      ///
      /// this function will check each specified property to see if it contains
      /// the search substring, returning true if a match was round. case-sensitive
      /// search is used (for now)
      /// 
      bool operator()(const T& rec) const
      {
         auto field_to_str = Overloaded{
            [this](std::string_view val)  -> bool
            { 
               return boost::icontains(val, search_value);
            },
            [this](NullableDouble val)
            {
               return val ? std::format("{}", *val).contains(search_value)
                          : false;
            },
            [this](NullableShort val)
            {
               return val ? std::format("{}", *val).contains(search_value)
                          : false;
            },            
            [this](auto val)               
            { 
               return std::format("{}", val).contains(search_value); 
            }
         };

         for (auto prop : search_props)
         {
            auto maybe_val = rec.getProperty(prop);
            if (maybe_val.has_value())
            {
               if (std::visit(field_to_str, maybe_val.value()))
                  return true; 
            }              
         }
         return false;
      }

   };


} // namespace ctb::data