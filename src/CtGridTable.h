#pragma once

#include "cts/Error.h"

#include <wx/grid.h>

namespace cts::data
{

   class CtGridTable : public wxGridTableBase
   {
   public:


      int GetNumberRows() override;
      int GetNumberCols() override;

      wxString GetValue(int row, int col) override;
      void SetValue(int row, int col, const wxString& value) override;

   protected:

   };

}  // namespace cts::data
