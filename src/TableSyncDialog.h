/*********************************************************************
 * @file       TableSyncDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "generated/TableSyncDlgBase.h"

namespace cts
{
   class TableSyncDialog : public TableSyncDlgBase
   {
   public:
      /// @brief  ctor for two-phase window creation, requires manually calling Create()
      TableSyncDialog() {}

      // @brief ctor for implicit window creation, no need to call create.
      TableSyncDialog(wxWindow* parent);

      /// @brief create the window object
      ///
      /// this should only be called if this object was default-constructed
      bool Create(wxWindow* parent);

   protected:
      void OnInitDialog(wxInitDialogEvent& event);
      void onOkUpdateUI(wxUpdateUIEvent& event);
   };

}
