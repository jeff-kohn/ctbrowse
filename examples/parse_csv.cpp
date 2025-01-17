#include "cts/constants.h"
#include "cts/data/WineList.h"
#include "cts/Error.h"

#include <cassert>
#include <deque>
#include <print>
#include <string>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4365 4464 4702)
#include "cts/external/csv.hpp"
#pragma warning(pop)


int main()
{
   using namespace std::literals;
   using namespace cts::data;

   try
   {
      csv::CSVReader reader{ R"(C:\Users\jkohn\AppData\Roaming\cts_win\List.csv)" };
      std::deque<WineListEntry> wines{};
      WineListEntry entry{};
      for (csv::CSVRow& row : reader)
      {
         if (entry.parse(row))
            wines.emplace_back(std::move(entry));
         else
            std::println("Skipping invalid row:\r\n\t{}\r\n", row.to_json());
      }
   }
   catch (std::exception& ex)
   {
      std::println("\r\nException occurred:{}\r\n", ex.what());
   }
}
