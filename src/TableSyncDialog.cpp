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

      return false;
   }


   void TableSyncDialog::OnInitDialog(wxInitDialogEvent& event)
   {
      // populate table name selection list
      auto table_names = magic_enum::enum_names<CellarTrackerDownload::Table>();
      m_table_selection_ctrl->InsertItems(wxToArrayString(table_names), 0);

      // populate data format combo
      auto format_names = magic_enum::enum_names<CellarTrackerDownload::Format>();
      m_data_format_ctrl->Insert(wxToArrayString(format_names), 0);
   }

}  // namespace cts
