/*******************************************************************
 * @file functors.h
 *
 * @brief Header file for
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

namespace ctb
{
   /// @brief a functor object that is overloaded for multiple types 
   template < typename... Ts >
   struct Overloaded : Ts...
   {
      using Ts:: operator()...;
   };

}
