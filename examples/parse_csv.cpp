/*******************************************************************
 * @file parse_csv.cpp
 *
 * @brief example program shwoing how to parse a table row from
 *        a CellarTracker CSV file into the corrsponding C++ class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *
 *******************************************************************/
#include "ctb/constants.h"
#include "ctb/WineListTraits.h"
#include "ctb/Error.h"

#include <cassert>
#include <deque>
#include <print>
#include <string>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4365 4464 4702)
#include "external/csv.hpp"
#pragma warning(pop)


int main()
{
   using namespace std::literals;
   using namespace ctb;

   try
   {
      csv::CSVReader reader{ R"(C:\Users\jkohn\AppData\Roaming\cts_win\List.csv)" };
      std::deque<WineListRecord> wines{};
      WineListRecord rec{};
      for (csv::CSVRow& row : reader)
      {
         rec.parse(row);
         wines.emplace_back(std::move(rec));
      }
   }
   catch (std::exception& ex)
   {
      std::println("\r\nException occurred:{}\r\n", ex.what());
   }
}
