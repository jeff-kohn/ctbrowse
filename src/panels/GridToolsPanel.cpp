#include "GridToolsPanel.h"

#include "App.h"


namespace ctb
{


   GridToolsPanel::GridToolsPanel(wxWindow* parent, CellarTrackerGrid* grid) : m_grid{ grid }
   {
      assert(m_grid);
      if (!m_grid)
         throw Error{ constants::ERROR_NULL_POINTER, Error::Category::ArgumentError };

      assert(parent);
      Create(parent);
   }


   bool GridToolsPanel::Create(wxWindow* parent)
   {
      if (!wxPanel::Create(parent))
         return false;

      createImpl();
      m_timer.SetOwner(this);
      Bind(wxEVT_TIMER, &GridToolsPanel::onSearchTimer, this);

      return true;
   }


   void GridToolsPanel::onClearSearchClicked(wxCommandEvent& event)
   {
      m_search_ctrl->ChangeValue("");
      doFilter();
   }


   void GridToolsPanel::onSearchTextChanged(wxCommandEvent& event)
   {
      // we'll trigger the search after they've stopped typing for ~500ms
      static constexpr int TIMER_INTERVAL{ 500 }; 
      m_timer.StartOnce(TIMER_INTERVAL);
   }


   void GridToolsPanel::onSearchTimer([[maybe_unused]] wxTimerEvent& event)
   {
      doFilter();
   }

   void GridToolsPanel::doFilter()
   {
      try
      {
         if (!TransferDataFromWindow())
         {
            wxGetApp().displayErrorMessage(constants::ERROR_DIALOG_TRANSFER_FAILED);
            return;
         }
         m_grid->filterBySubstring(m_search_value.wx_str());
      }
      catch (Error& e)
      {
         wxGetApp().displayErrorMessage(e);
      }
   }


   // as ugly as this code is, having to tweak it from scratch for every little UI change would be a nightmare.
   // we copy/paste it from the generated base-class's Create() function whenever we make changes in the GUI 
   // editor. That way we don't have to inherit from a base class and use virtual fn's for message handlers,
   // we can also change the type for certain members (just have to create them before calling this)
   void GridToolsPanel::createImpl()
   {
      SetMaxSize(ConvertDialogToPixels(wxSize(-1, 150)));
      auto* panel_sizer = new wxBoxSizer(wxVERTICAL);

      auto* static_text = new wxStaticText(this, wxID_ANY, "&Search:");
      panel_sizer->Add(static_text, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, FromDIP(wxSize(6, -1)).x));

      auto* box_sizer2 = new wxBoxSizer(wxHORIZONTAL);

      m_search_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
      m_search_ctrl->SetValidator(wxTextValidator(wxFILTER_NONE, &m_search_value));
      box_sizer2->Add(m_search_ctrl, wxSizerFlags(2).Expand().Border(wxALL));

      auto* clear_search_btn = new wxButton(this, wxID_ANY, "X");
      clear_search_btn->SetMaxSize(ConvertDialogToPixels(wxSize(10, -1)));
      box_sizer2->Add(clear_search_btn,
         wxSizerFlags().Center().Border(wxRIGHT|wxTOP|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

      panel_sizer->Add(box_sizer2, wxSizerFlags().Expand());

      SetSizer(panel_sizer);
      Fit();


      // Event handlers
      clear_search_btn->Bind(wxEVT_BUTTON, &GridToolsPanel::onClearSearchClicked, this);
      m_search_ctrl->Bind(wxEVT_TEXT, &GridToolsPanel::onSearchTextChanged, this);

   }

}