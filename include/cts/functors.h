#pragma once

namespace cts
{
   /// @brief a functor object that is overloaded for multiple types 
   template < typename... Ts >
   struct Overloaded : Ts...
   {
      using Ts:: operator()...;
   };

}
