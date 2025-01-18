#include "CtGridTable.h"

#include "cts/functors.h"
#include <cassert>

namespace cts::data
{
   using std::ssize;

   auto FieldToStr = Overloaded{
      [](uint64_t val)           { return wxString{ std::format("{}", val)};   },
      [](uint16_t val)           { return wxString{ std::format("{}", val)};   },
      [](double val)             { return wxString{ std::format("{}", val)};   },
      [](const std::string& val) { return wxString{ std::format("{}", val)};   }
   };

   
   wxString CtGridTable::GetValue(int row, int col)
   {
      if (row >= ssize(m_data))
      {
         assert(false);
         return wxString{};
      }

      return m_data[static_cast<size_t>(row)][col]
               .and_then([] (auto val) -> std::expected<wxString, Error>
                  {
                     // todo: check perf, may want to use manual visit
                     return std::visit(FieldToStr, val);
                  })
               .value_or(wxString{});
   } 

   void CtGridTable::SetValue(int row, int col, const wxString& value)
   {
      throw Error("This dataset does not support editing.");
   }

}
