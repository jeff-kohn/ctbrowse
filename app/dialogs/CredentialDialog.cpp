#include "CredentialDialog.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/secretstore.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

namespace ctb::app
{


   CredentialDialog::CredentialDialog(wxWindow* parent, std::string_view credential_name, std::string_view prompt_msg, bool allow_save) : 
      m_cred_name{ credential_name },
      m_prompt_msg{ prompt_msg.data(), prompt_msg.size() },
      m_allow_save{ allow_save }
   {
      auto title = ctb::format(constants::FMT_CREDENTIALDLG_LBL_TITLE, credential_name);

      if (!Create(parent, wxID_ANY, title))
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };

      init();
   }

   CredentialDialog::~CredentialDialog()
   {
      wxSecretValue::WipeString(m_username_val);
      wxSecretValue::WipeString(m_password_val);
   }

   CredentialWrapper CredentialDialog::getCredential()
   {
      TransferDataFromWindow();
      return CredentialWrapper { std::move(m_cred_name), m_username_val.utf8_string(), m_password_val.utf8_string(), m_save_requested };
   }


   void CredentialDialog::init()
   {
      // set up main sizer and prompt message.
      auto* dlg_sizer = new wxBoxSizer{ wxVERTICAL };
      dlg_sizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

      // prompt message to display above the login form
      const auto prompt_size = ConvertDialogToPixels(wxSize{155, 155});
      dlg_sizer->Add(CreateTextSizer(m_prompt_msg, prompt_size.x), wxSizerFlags{}.Border());

      // we want labels and text fields in 2x2 grid but use 2 vertical sizers in a horizontal
      // because a grid sizer would give equal space to each column and look bad.
      auto* form_sizer = new wxBoxSizer{ wxHORIZONTAL };

      // First column is labels
      auto* label_col_sizer = new wxBoxSizer{ wxVERTICAL };
      
      // username
      auto* username_label = new wxStaticText{ this, wxID_ANY, constants::CREDENTIALDLG_LBL_USERNAME };
      label_col_sizer->Add(username_label, wxSizerFlags{}.Border(wxALL));
      
      // password
      auto* password_lbl = new wxStaticText{ this, wxID_ANY, constants::CREDENTIALDLG_LBL_PASSWORD };
      label_col_sizer->Add(password_lbl, wxSizerFlags{}.Border(wxALL));

      form_sizer->Add(label_col_sizer, wxSizerFlags{});

      // second column is text fields
      auto* text_col_sizer = new wxBoxSizer{ wxVERTICAL };
      
      // username
      auto* username_text = new wxTextCtrl{ this, wxID_ANY, wxEmptyString };
      username_text->SetValidator(wxTextValidator{ wxFILTER_NONE, &m_username_val });
      text_col_sizer->Add(username_text, wxSizerFlags{}.Border(wxALL));
      
      // password
      auto* password_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
      password_text->SetValidator(wxTextValidator{ wxFILTER_NONE, &m_password_val });
      text_col_sizer->Add(password_text, wxSizerFlags{}.Border(wxALL));

      dlg_sizer->Add(form_sizer, wxSizerFlags{});
      if (m_allow_save)
      {
         auto* check_box = new wxCheckBox(this, wxID_ANY, constants::CREDENTIALDLG_LBL_SAVE);
         check_box->SetValidator(wxGenericValidator(&m_save_requested));
         text_col_sizer->Add(check_box, wxSizerFlags{}.Left().Border(wxALL));
      }

      form_sizer->Add(text_col_sizer, wxSizerFlags{});
      auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
      dlg_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags{}.Expand().Border(wxALL));

      SetSizerAndFit(dlg_sizer);
      Centre(wxBOTH);
      username_text->SetFocus();
   }
}