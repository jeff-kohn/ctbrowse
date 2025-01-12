/*********************************************************************
 * @file       TableSyncDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "TableSyncDialog.h"
#include "wx_helpers.h"

#include "cts/CellarTrackerDownload.h"

#include "magic_enum/magic_enum.hpp"

namespace cts
{

   TableSyncDialog::TableSyncDialog(wxWindow* parent)
   {
      Create(parent);
   }


   bool TableSyncDialog::Create(wxWindow* parent)
   {
      if (!TableSyncDlgBase::Create(parent))
         return false;

      Bind(wxEVT_INIT_DIALOG, &TableSyncDialog::OnInitDialog, this);
      Bind(wxEVT_UPDATE_UI, &TableSyncDialog::onOkUpdateUI, this, wxID_OK);

      return false;
   }


   void TableSyncDialog::OnInitDialog(wxInitDialogEvent& event)
   {
      // populate table name selection list
      auto table_names = magic_enum::enum_names<CellarTrackerDownload::Table>();
      m_table_selection_ctrl->InsertItems(wxToArrayString(table_names), 0);
   }


   void TableSyncDialog::onOkUpdateUI([[maybe_unused]] wxUpdateUIEvent& event)
   {
      wxArrayInt dummy{};
      event.Enable(m_table_selection_ctrl->GetCheckedItems(dummy));
   }


}  // namespace cts
