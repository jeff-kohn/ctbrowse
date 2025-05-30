/*******************************************************************
 * @file parse_csv.cpp
 *
 * @brief example program shwoing how to parse a table row from
 *        a CellarTracker CSV file into the corrsponding C++ class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *
 *******************************************************************/
#include <ctb/constants.h>
#include <ctb/Error.h>
#include <ctb/tables/WineListTraits.h>

#include <cassert>
#include <print>
#include <string>

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
      WineListTable wines{};
      WineListTable::value_type rec{};
      for (csv::CSVRow& row : reader)
      {
         rec.parseRow(row);
         wines.emplace_back(std::move(rec));
      }
   }
   catch (std::exception& ex)
   {
      std::println("\r\nException occurred:{}\r\n", ex.what());
   }
}
