/*********************************************************************
 * @file       ExportDatasetDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "CtDatasetExportOptions.h"

#include <ctb/interfaces/IDataset.h>
#include <ctb/tables/CtSchema.h>
#include <ctb/tables/table_data.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/dialog.h>

#include <set>
#include <vector>

class wxCheckListBox;
class wxTextCtrl;


namespace ctb::app
{

   /// @brief class implementing a dialog for syncing data from CellarTraker.com
   ///
   class ExportDatasetDialog final : public wxDialog
   {
   public:
      ExportDatasetDialog(wxWindow* parent, DatasetPtr dataset);

      enum class DataSelection
      {
         Filtered,
         Raw,
      };


      template<typename SelfT>
      auto&& exportOptions(this SelfT&& self) noexcept
      {
         return std::forward<SelfT>(self).m_export_options;
      }

      bool TransferDataToWindow() override;
      bool TransferDataFromWindow() override;

   private:
      void init();
      void onOkClicked(wxCommandEvent& event);
      void onOkUpdateUI(wxUpdateUIEvent& event);
      void onBrowseClicked(wxCommandEvent& event);

      void getSelectedProps() noexcept(false);              // throws on empty
      void getTargetFilePath() noexcept(false);             // throws on empty
      auto clientDataToProp(unsigned int index) -> CtProp;

      using PropSet = std::set<CtProp>;

      wxTextCtrl*            m_path_text_ctl{};
      wxCheckListBox*        m_checklist_ctl{};
      CtDatasetExportOptions m_export_options{};
      PropSet                m_dataset_props{};
      bool                   m_save_default_flag{ false };
   };


}   // namespace ctb::app
