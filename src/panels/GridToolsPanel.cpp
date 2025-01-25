#include "GridToolsPanel.h"

#include "App.h"


namespace ctb
{


   GridToolsPanel::GridToolsPanel(wxWindow* parent_ptr)
   {
      Create(parent_ptr);
   }


   bool GridToolsPanel::Create(wxWindow* parent_ptr)
   {
      if (!wxPanel::Create(parent_ptr))
         return false;

      createImpl();
      return true;
   }


   void GridToolsPanel::onClearSearchClicked(wxCommandEvent& event)
   {
      m_search_ctrl->SetValue("");
   }


   void GridToolsPanel::onSearchTextChanged(wxCommandEvent& event)
   {
      event.Skip();
   }


   // as ugly as this code is, having to tweak it from scratch for every little UI change would be a nightmare.
   // we copy/paste it from the generated base-class's Create() function whenever we make changes in the GUI 
   // editor. That way we don't have to inherit from a base class and use virtual fn's for message handlers,
   // we can also change the type for certain members (just have to create them before calling this)
   void GridToolsPanel::createImpl()
   {
      auto* panel_sizer = new wxBoxSizer(wxVERTICAL);

      auto* static_text = new wxStaticText(this, wxID_ANY, "&Search:");
      panel_sizer->Add(static_text,
         wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

      auto* box_sizer2 = new wxBoxSizer(wxHORIZONTAL);

      m_search_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
      m_search_ctrl->SetValidator(wxTextValidator(wxFILTER_NONE, &m_search_value));
      box_sizer2->Add(m_search_ctrl,
         wxSizerFlags(2).Bottom().Expand().Border(wxLEFT|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

      auto* clear_search_btn = new wxButton(this, wxID_ANY, "X");
      clear_search_btn->SetMaxSize(ConvertDialogToPixels(wxSize(10, -1)));
      box_sizer2->Add(clear_search_btn,
         wxSizerFlags().Center().Border(wxRIGHT|wxTOP|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

      panel_sizer->Add(box_sizer2, wxSizerFlags().Expand());

      SetSizerAndFit(panel_sizer);

      clear_search_btn->Bind(wxEVT_BUTTON, &GridToolsPanel::onClearSearchClicked, this);
      m_search_ctrl->Bind(wxEVT_TEXT, &GridToolsPanel::onSearchTextChanged, this);
   }

}