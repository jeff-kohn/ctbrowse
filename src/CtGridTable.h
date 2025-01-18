#pragma once

#include "cts/Error.h"
#include "cts/data/WineList.h"

#include <wx/grid.h>

namespace cts::data
{

   class CtGridTable : public wxGridTableBase
   {
   public:
      CtGridTable(WineListData&& data) : m_data(std::move(data)){}

      int GetNumberRows() override { return std::ssize(m_data);  }
      int GetNumberCols() override { return static_cast<int>(WineListEntry::maxPropIndex() + 1); }

      wxString GetValue(int row, int col) override;
      void SetValue(int row, int col, const wxString& value) override;

   protected:
      WineListData m_data{};
   };

}  // namespace cts::data
