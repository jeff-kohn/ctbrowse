
#pragma once

#include "generated/TableSyncDlgBase.h"

namespace cts
{
   class TableSyncDialog : public TableSyncDlgBase
   {
   public:
      TableSyncDialog() {}  // If you use this constructor, you must call Create(parent)
      TableSyncDialog(wxWindow* parent);

      bool Create(wxWindow* parent);

      void OnInitDialog(wxInitDialogEvent& event);
   };

}
