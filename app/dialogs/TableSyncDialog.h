/*********************************************************************
 * @file       TableSyncDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include <ctb/table/table_data.h>

#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <vector>


namespace ctb::app
{

   /// @brief class implementing a dialog for syncing data from CellarTraker.com
   ///
   class TableSyncDialog final : public wxDialog
   {
   public:
      /// @brief  ctor for two-phase window creation, requires manually calling Create()
      ///
      TableSyncDialog() {}

      /// @brief ctor for implicit window creation, no need to call create.
      ///
      TableSyncDialog(wxWindow* parent);

      /// @brief create the window object
      ///
      /// this should only be called if this object was default-constructed
      ///
      bool Create(wxWindow* parent);

      /// @brief set the list of tables that should be selected for download.
      ///
      template <rng::input_range RngT>
      void selectTables(RngT&& values) requires std::is_same_v<rng::range_value_t<RngT>, TableId>
      {
         m_table_selection_val = values | vws::transform([] (TableId tbl) { return magic_enum::enum_index(tbl); })
                                        | rng::to<wxArrayString>();
      }

      /// @brief retrieve the list of tables the user selected for download
      ///
      [[nodiscard]] std::vector<TableId> selectedTables() const;

      /// @brief  indicates whether the user checked "Save as Default" in the dialog
      ///
      bool saveAsDefault() const noexcept { return m_save_default_val; }

      /// @brief indicates whether the user checked "Automatically Sync on Startup"
      ///
      bool syncOnStartup() const noexcept { return m_startup_sync_val; }

   protected:
      bool           m_save_default_val{ false };
      bool           m_startup_sync_val{ false };
      wxCheckBox*    m_save_default_ctrl{};  
      wxCheckBox*    m_startup_sync_ctrl{};  
      wxArrayInt     m_table_selection_val{};
      wxCheckListBox* m_table_selection_ctrl;

      void onOkUpdateUI(wxUpdateUIEvent& event);
      void onOkClicked(wxCommandEvent& event);
      void onDeselectAll(wxCommandEvent& event);
      void onDeselectAllUpdateUI(wxUpdateUIEvent& event);
      void onSelectAll(wxCommandEvent& event);
      void onSelectAllUpdateUI(wxUpdateUIEvent& event);
      void createImpl();

      size_t checkedTableCount() const
      {
         wxArrayInt dummy{};
         m_table_selection_ctrl->GetCheckedItems(dummy);
         return dummy.size();
      }

   };

}  // namespace ctb::app
