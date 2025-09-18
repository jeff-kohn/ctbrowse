/*******************************************************************
 * @file parse_csv.cpp
 *
 * @brief example program showing how to parse a table row from
 *        a CellarTracker CSV file into the corresponding C++ class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *
 *******************************************************************/
#include <ctb/tables/WineListTraits.h>

#include <glaze/glaze.hpp>

#include <fstream>
#include <print>
#include <unordered_map>
#include <string>
#include <string_view>

using std::string_view;




struct record
{
   std::vector<string_view> values{};
};

struct csv_table
{
   std::vector<record> records{};
};


int main()
{
   using namespace std::literals;
   using namespace ctb;

   try
   {
      fs::path file_path{ "%APPDATA%/ctBrowse for Windows/Tables/List.csv" };

      csv_table table{};
      auto size = std::filesystem::file_size(file_path);
      std::string buffer(size, '\0');
      std::ifstream in(file_path);
      in.read(&buffer[0], static_cast<std::streamsize>(size));

      auto ec = glz::read_csv(table, buffer);
      if (ec)
         throw Error{ glz::format_error(ec, buffer), Error::Category::ParseError };

      auto record = table.records[0];
   }
   catch (std::exception& ex)
   {
      std::println("\r\nException occurred:{}\r\n", ex.what());
   }
}
