/*********************************************************************
 * @file       ExportDatasetDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
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

      static inline constexpr std::string_view DEFAULT_EXTENSION      = "txt";
      static inline constexpr std::string_view DEFAULT_EXTENSION_NAME = "Tab Separated Values";

      enum class DataSelection
      {
         Filtered,
         Raw,
      };


      std::vector<CtProp> selectedProperties() const
      {
         return m_selected_props | rng::to<std::vector>();
      }
      void selectProperties(std::span<CtProp> props);


      std::string targetFilePath() const
      {
         return m_target_path.generic_string();
      }
      void setTargetFilePath(std::string_view path);

      bool TransferDataToWindow() override;
      bool TransferDataFromWindow() override;

   private:
      void init();
      void onOkClicked(wxCommandEvent& event);
      void onOkUpdateUI(wxUpdateUIEvent& event);
      void onBrowseClicked(wxCommandEvent& event);
      void onBrowseUpdateUI(wxUpdateUIEvent& event);

      void getSelectedProps() noexcept(false);    // throws on empty
      void getTargetFilePath() noexcept(false);   // throws on empty
      CtProp clientDataToProp(unsigned int index);
      void loadDefaults();
      void saveDefaults();
      void applyDefaults(std::string_view dataset_name);

      using PropSet = std::set<CtProp>;

      wxTextCtrl*     m_path_text_ctl{};
      wxCheckListBox* m_checklist_ctl{};
      PropSet         m_dataset_props{};
      PropSet         m_selected_props{};
      bool            m_save_default_flag{ false };
      std::string     m_dataset_name{};
      fs::path        m_target_path{};
   };


}   // namespace ctb::app
