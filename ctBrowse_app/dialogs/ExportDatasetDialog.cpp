#include "ExportDatasetDialog.h"

#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

namespace ctb::app
{
   
   ExportDatasetDialog::ExportDatasetDialog(wxWindow* parent, DatasetPtr dataset)
      : wxDialog{
           parent,
           wxID_ANY, //
           constants::TITLE_EXPORT_DATA,
           wxDefaultPosition,
           wxDefaultSize,
           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
   },
        m_export_options{ dataset ? getDefaultExportOptions(dataset) : CtDatasetExportOptions{} },
        m_dataset_props{ dataset ? PropSet{ std::from_range, dataset->getAvailableFields() } : PropSet{} }

   {
      init();
   }


   bool ExportDatasetDialog::TransferDataToWindow()
   {
      wxWindowUpdateLocker lock_refresh{ this };

      // handle transfer for non-validator members before calling base.
      m_path_text_ctl->SetValue(m_export_options.target_path.c_str());

      rng::for_each(vws::iota(0U, m_checklist_ctl->GetCount()),
                    [this](unsigned int idx)
                    {
                       auto prop_data = clientDataToProp(idx);
                       m_checklist_ctl->Check(idx, m_export_options.props_to_export.contains(prop_data));
                    });

      return wxDialog::TransferDataToWindow();
   }


   bool ExportDatasetDialog::TransferDataFromWindow()
   {
      // validate path and prop selection and update members before calling base
      // class to update validators
      getTargetFilePath();
      getSelectedProps();
      return wxDialog::TransferDataFromWindow();
   }


   void ExportDatasetDialog::init()
   {
      // clang-format off
      using namespace constants;

      constexpr auto SPACER_HEIGHT      = 5;
      constexpr auto PATH_CTL_MIN_WIDTH = 300;   // DIPs

      // contains top_row_sizer and target_path_sizer
      auto* dlg_sizer = new wxBoxSizer{wxVERTICAL};
      dlg_sizer->AddSpacer(SPACER_HEIGHT);

      // this static box sizer contains the property check-list
      auto* props_sizer = new wxStaticBoxSizer{ wxVERTICAL, this, LBL_SELECTED_PROPERTIES};
      dlg_sizer->Add(props_sizer, wxSizerFlags{}.Border().DoubleHorzBorder().Expand());

      // check list box control for export property selection
      m_checklist_ctl = new wxCheckListBox(props_sizer->GetStaticBox(), wxID_ANY);//, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE);
      props_sizer->Add(m_checklist_ctl,wxSizerFlags{}.Expand().DoubleBorder());
      for (auto prop : m_dataset_props)
      {
         m_checklist_ctl->Append(wxEnumToString(prop), wxUIntToPtr(std::to_underlying(prop)));
      }

      // radio group label and option buttons
      //dlg_sizer->AddSpacer(SPACER_HEIGHT);
      //auto choices = enum_names<DataSelection> | vws::transform(&wxFromSV) | rng::to<std::vector>();
      //auto* radio_box_ctl = new wxRadioBox{ this, wxID_ANY, LBL_DATA_OPTIONS, wxDefaultPosition, wxDefaultSize, static_cast<int>(choices.size()), choices.data(), 0, wxRA_SPECIFY_COLS}; 
      //dlg_sizer->Add(radio_box_ctl, wxSizerFlags{}.Expand().DoubleHorzBorder());

      // target path label
      auto* static_text = new wxStaticText(this, wxID_ANY, constants::LBL_TARGET_FILE_PATH);
      dlg_sizer->Add(static_text, wxSizerFlags{}.DoubleBorder(wxLEFT | wxTOP));

      // Sizer for the target path UI elements
      auto* target_path_sizer = new wxBoxSizer{wxHORIZONTAL};
      dlg_sizer->Add(target_path_sizer, wxSizerFlags{}.Border().Expand());

      // Textbox for target file path.
      m_path_text_ctl = new wxTextCtrl(this, wxID_ANY, m_export_options.target_path.c_str());
      m_path_text_ctl->SetMinSize(FromDIP(wxSize{ PATH_CTL_MIN_WIDTH, -1 }));
      target_path_sizer->Add(m_path_text_ctl, wxSizerFlags{1}.Border(wxLEFT|wxBOTTOM));

      // Browse button for target path
      auto* browse_btn_ctl = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_FILE_OPEN, wxART_BUTTON));
      target_path_sizer->Add(browse_btn_ctl, wxSizerFlags{}.Center().Border(wxRIGHT|wxBOTTOM));
      // we want to force the button to be square and the same height as the edit.
      const int h = m_path_text_ctl->GetBestSize().y;
      browse_btn_ctl->SetMinSize({h, h});

      auto* save_default_checkbox = new wxCheckBox{ this, wxID_ANY, constants::LBL_SAVE_EXPORT_DATASET_DEFAULT };
      save_default_checkbox->SetValidator(wxGenericValidator{ &m_save_default_flag });
      dlg_sizer->Add(save_default_checkbox, wxSizerFlags().DoubleBorder());
      dlg_sizer->Add(CreateSeparatedSizer(CreateStdDialogButtonSizer(wxOK | wxCANCEL)), wxSizerFlags().Expand().Border(wxALL));

      SetSizerAndFit(dlg_sizer);
      Centre(wxBOTH);
      m_checklist_ctl->SetFocus();
      m_checklist_ctl->SetSelection(0);

      // Event bindings
      Bind(wxEVT_UPDATE_UI, &ExportDatasetDialog::onOkUpdateUI, this, wxID_OK);
      Bind(wxEVT_BUTTON, &ExportDatasetDialog::onOkClicked, this, wxID_OK);
      browse_btn_ctl->Bind(wxEVT_BUTTON, &ExportDatasetDialog::onBrowseClicked, this);
   }


   void ExportDatasetDialog::onOkClicked(wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow(); // for the checkbox validator, we retrieve the other props manually.

         if (m_save_default_flag)
            saveDefaultExportOptions(m_export_options);

         event.Skip();
      }
      catch (...)
      {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void ExportDatasetDialog::onOkUpdateUI(wxUpdateUIEvent& event)
   {
      try
      {
         bool any_checked = rng::any_of(vws::iota(0U, m_checklist_ctl->GetCount()),
                                        [this](unsigned int i)
                                        {
                                           return m_checklist_ctl->IsChecked(i);
                                        });
        
         event.Enable(any_checked and !m_path_text_ctl->IsEmpty());
      }
      catch (...)
      {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void ExportDatasetDialog::onBrowseClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         getTargetFilePath();
      }
      catch (...) // NOLINT
      {
         // don't care 
      }
      wxFileDialog dialog{ this,
                           ctb::format(constants::FMT_EXPORT_DATASET_TITLE, m_export_options.collection_name),
                           m_export_options.target_path.parent_path().generic_string(),
                           m_export_options.target_path.filename().c_str(),
                           ctb::format("{0} (*.{1})|*.{1}|All files (*.*)|*.*", CtDatasetExportOptions::DEFAULT_EXTENSION_NAME, CtDatasetExportOptions::DEFAULT_EXTENSION).c_str(),
                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT };

      if (dialog.ShowModal() == wxID_CANCEL) return;

      m_path_text_ctl->SetValue(dialog.GetPath());
      m_path_text_ctl->SelectAll();
      m_path_text_ctl->SetScrollPos(0,0);
      m_path_text_ctl->SetFocusFromKbd();
   }


   void ExportDatasetDialog::getSelectedProps() noexcept(false)
   {

      m_export_options.props_to_export = vws::iota(0U, m_checklist_ctl->GetCount())
                       | vws::filter(std::bind_front(&wxCheckListBox::IsChecked, m_checklist_ctl))
                       | vws::transform(
                            [this](unsigned int i)
                            {
                               return clientDataToProp(i);
                            })
                       | std::ranges::to<std::set>();

      if (m_export_options.props_to_export.empty())
      {
         throw ctb::Error{ constants::ERROR_STR_NO_EXPORT_PROPS_SELECTED, Error::Category::GeneralError };
      }
   }


   void ExportDatasetDialog::getTargetFilePath() noexcept(false)
   {
      wxFileName file_name{ m_path_text_ctl->GetValue().Trim() };
      file_name.MakeAbsolute(wxGetApp().getDataFolder(AppFolder::Exports).generic_string());

      if (!file_name.IsOk()) throw ctb::Error{ Error::Category::FileError, "\"{}\" is not a valid path.", file_name.GetFullPath().utf8_string() };

      auto parent_folder = file_name.GetPath().utf8_string();
      if (!wxFileName::DirExists(parent_folder.c_str()))
         throw ctb::Error{ Error::Category::FileError, "Folder \"{}\" does not exist.",  parent_folder };

      m_export_options.target_path = file_name.GetFullPath().utf8_string();
   }

   CtProp ExportDatasetDialog::clientDataToProp(unsigned int index)
   {
      assert(m_checklist_ctl);
      return static_cast<CtProp>(wxPtrToUInt(m_checklist_ctl->GetClientData(index)));
   }


}   // namespace ctb::app
