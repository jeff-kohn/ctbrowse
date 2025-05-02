/*********************************************************************
 * @file       TableSyncDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "dialogs/TableSyncDialog.h"
#include "CtCredentialManager.h"
#include "wx_helpers.h"

#include <magic_enum/magic_enum.hpp>

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

#include <algorithm>
#include <sstream>

namespace ctb::app
{
   using namespace magic_enum;

   inline constexpr auto ENUM_DELIMETER = ';';

   namespace 
   {

      /// @brief serialize a range of integer values to a delimited string
      template<rng::input_range Rng, typename I = rng::range_value_t<Rng> >
         requires std::is_same_v<std::decay_t<rng::range_value_t<Rng>>, I> and std::is_integral_v<I>
      std::string serializeIntegrals(Rng rg, char delim = ENUM_DELIMETER)
      {
         std::ostringstream str{};
         for (I i : rg)
         {
            str << i << delim;
         }
         return str.str();
      }
   }



   TableSyncDialog::TableSyncDialog(wxWindow* parent)
   {
      Create(parent);
   }


   bool TableSyncDialog::Create(wxWindow* parent)
   {
      try{
         // give base class a chance set up controls etc
         if (!wxDialog::Create(parent, wxID_ANY, constants::TITLE_DOWNLOAD_DATA))
            return false;

         createImpl();

         // message handlers
         Bind(wxEVT_UPDATE_UI, &TableSyncDialog::onOkUpdateUI, this, wxID_OK);
         Bind(wxEVT_BUTTON, &TableSyncDialog::onOkClicked, this, wxID_OK);

         // populate table name selection list
         constexpr auto table_descriptions = TableDescriptions | vws::values;
         m_table_selection_ctrl->InsertItems(wxToArrayString(table_descriptions), 0);

         // need to read some defaults from config settings.
         auto cfg = wxGetApp().getConfig(constants::CONFIG_PATH_PREFERENCE_DATASYNC);

         // default-selected tables are stored as a string of enum values (e.g int values not names)
         // delimited by ENUM_DELIMTER. The default value is the table enum value 0 (List)
         m_table_selection_val = std::string_view{ cfg->Read(constants::CONFIG_VALUE_DEFAULT_SYNC_TABLES, "0").wx_str() } // read the config value
            | vws::split(ENUM_DELIMETER)                                                                                 // split by token ';'
            | vws::transform([] (auto subrange) { return std::string_view(subrange.begin(), subrange.end()); })          // convert subranges to string_view's
            | vws::transform([] (std::string_view sv) { return from_str<int>(sv); })                                     // convert string view to from_chars() result
            | vws::filter([] (auto opt) { return opt.has_value(); })                                                     // filter out results that have no value
            | vws::transform([](auto opt) { return opt.value(); })                                                       // retrieve actual value from remaining results
            | rng::to<wxArrayInt>();                                                                                     // convert to array

         // whether the "Sync on Startup" box should be checked.
         m_startup_sync_val = cfg->ReadBool(constants::CONFIG_VALUE_SYNC_ON_STARTUP, false);

         TransferDataToWindow();
         return true;
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
      return false;
   }


   std::vector<TableId> TableSyncDialog::selectedTables() const
   {
      using namespace vws;
      using EnumT = TableId;

      // convert our internal list of ints to the actual enum, need to filter out
      // values that don't map to an enum (should never happen, but best to be
      // prepared since alternative is UB)
      return all(m_table_selection_val)
         | transform([] (int val) { return enum_cast<EnumT>(val); }) // convert to optional<EnumT>
         | filter([](auto maybe_enum) { return maybe_enum.has_value(); })        // filter only valid values
         | transform([](auto maybe_enum) { return maybe_enum.value(); })         // retrieve actual value from optionals
         | rng::to<std::vector>();
   }


   void TableSyncDialog::onOkClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         if (!TransferDataFromWindow())
         {
            wxGetApp().displayErrorMessage(constants::ERROR_STR_DIALOG_TRANSFER_FAILED, false);
            return;
         }

         // Save relevant settings to config
         auto cfg = wxGetApp().getConfig(constants::CONFIG_PATH_PREFERENCE_DATASYNC);
         cfg->Write(wxString::FromUTF8(constants::CONFIG_VALUE_SYNC_ON_STARTUP), m_startup_sync_val);
         if (m_save_default_val)
         {
            cfg->Write(wxString::FromUTF8(constants::CONFIG_VALUE_DEFAULT_SYNC_TABLES), wxString::FromUTF8(serializeIntegrals(vws::all(m_table_selection_val))));
         }
         cfg->Flush();

         EndDialog(wxID_OK);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void TableSyncDialog::onDeselectAll([[maybe_unused]] wxCommandEvent& event)
   {
      for (auto idx = 0u; idx < m_table_selection_ctrl->GetCount(); ++idx)
      {
         m_table_selection_ctrl->Check(idx, false);
      }
   }


   void TableSyncDialog::onDeselectAllUpdateUI([[maybe_unused]] wxUpdateUIEvent & event)
   {
      // we already check for at least one selection for OK button, so just piggy back.
      onOkUpdateUI(event);
   }


   void TableSyncDialog::onSelectAll([[maybe_unused]] wxCommandEvent & event)
   {
      for (auto idx = 0u; idx < m_table_selection_ctrl->GetCount(); ++idx)
      {
         m_table_selection_ctrl->Check(idx, true);
      }
   }


   void TableSyncDialog::onSelectAllUpdateUI(wxUpdateUIEvent & event)
   {
      event.Enable(checkedTableCount() != m_table_selection_ctrl->GetCount());
   }


   void TableSyncDialog::onOkUpdateUI([[maybe_unused]] wxUpdateUIEvent& event)
   {
      // only enabled if at least one is checked
      event.Enable(checkedTableCount());
   }


   void TableSyncDialog::createImpl()
   {
      auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);

      auto* box_sizer2 = new wxBoxSizer(wxHORIZONTAL);

      auto* box_sizer3 = new wxBoxSizer(wxVERTICAL);

      auto* static_text2 = new wxStaticText(this, wxID_ANY, "&Tables to Download:");
      box_sizer3->Add(static_text2,
         wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

      m_table_selection_ctrl = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr,
         wxLB_EXTENDED);
      m_table_selection_ctrl->SetValidator(wxGenericValidator(&m_table_selection_val));
      m_table_selection_ctrl->SetMinSize(ConvertDialogToPixels(wxSize(112, 112)));
      box_sizer3->Add(m_table_selection_ctrl,
         wxSizerFlags().Border(wxLEFT|wxTOP|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

      m_startup_sync_ctrl = new wxCheckBox(this, wxID_ANY, "Sync on &Program Startup");
      m_startup_sync_ctrl->SetValidator(wxGenericValidator(&m_startup_sync_val));
      box_sizer3->Add(m_startup_sync_ctrl, wxSizerFlags().Border(wxALL));

      m_save_default_ctrl = new wxCheckBox(this, wxID_ANY, "Save as &Default");
      m_save_default_ctrl->SetValidator(wxGenericValidator(&m_save_default_val));
      box_sizer3->Add(m_save_default_ctrl, wxSizerFlags().Border(wxALL));

      box_sizer2->Add(box_sizer3, wxSizerFlags().Border(wxALL));

      auto* box_sizer = new wxBoxSizer(wxVERTICAL);

      box_sizer->AddSpacer(20);

      auto* m_btn_select_all = new wxButton(this, wxID_ANY, "Select &All");
      box_sizer->Add(m_btn_select_all, wxSizerFlags().Expand().Border(wxTOP, wxSizerFlags::GetDefaultBorder()));

      auto* btn_deselect_all = new wxButton(this, wxID_ANY, "&Deselect All");
      box_sizer->Add(btn_deselect_all, wxSizerFlags().Border(wxTOP|wxBOTTOM, FromDIP(wxSize(4, -1)).x));

      box_sizer2->Add(box_sizer, wxSizerFlags().Border(wxALL));

      dlg_sizer->Add(box_sizer2, wxSizerFlags().Expand().Border(wxALL));

      auto* std_buttons = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      dlg_sizer->Add(CreateSeparatedSizer(std_buttons), wxSizerFlags().Expand().Border(wxALL));

      SetSizerAndFit(dlg_sizer);
      Centre(wxBOTH);

      // Event handlers
      btn_deselect_all->Bind(wxEVT_BUTTON, &TableSyncDialog::onDeselectAll, this);
      m_btn_select_all->Bind(wxEVT_BUTTON, &TableSyncDialog::onSelectAll, this);
      btn_deselect_all->Bind(wxEVT_UPDATE_UI, &TableSyncDialog::onDeselectAllUpdateUI, this);
      m_btn_select_all->Bind(wxEVT_UPDATE_UI, &TableSyncDialog::onSelectAllUpdateUI, this);

   }

}  // namespace ctb::app
