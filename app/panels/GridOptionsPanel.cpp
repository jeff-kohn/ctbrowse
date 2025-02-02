#include "MainFrame.h"
#include "App.h"
#include "grids/GridTableBase.h"
#include "panels/GridOptionsPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

namespace ctb::app
{


   GridOptionsPanel::GridOptionsPanel(MainFrame* parent)
   {
      Create(parent);
   }


   bool GridOptionsPanel::Create(MainFrame* parent)
   {
      assert(parent);
      m_parent = parent;

      if (!wxPanel::Create(m_parent,wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
         return false;

      // panel shouldn't grow infinitely
      SetMaxSize(ConvertDialogToPixels(wxSize{ 125, constants::UNSPECIFIED }));

      // defines the rows of controls in our panel
      auto* top_sizer = new wxBoxSizer(wxVERTICAL);
      auto default_border = wxSizerFlags::GetDefaultBorder();

      // label for the sort combo
      auto* lbl_sort_by = new wxStaticText{ this, wxID_ANY, constants::LBL_SORT_BY };
      top_sizer->Add(lbl_sort_by, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

      m_sort_combo = new wxChoice(this, wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_idx));
      top_sizer->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, default_border));

      SetSizerAndFit(top_sizer);

      return true;
   }

   void GridOptionsPanel::populateSortOptions(GridTableBase::GridTablePtr grid)
   {
      using SortName = GridTableBase::SortOptionName;
      auto sort_options = vws::all(grid->availableSortOptions()) 
         | vws::transform([](const SortName& s) -> wxString 
            {  
               return wxString{s.sort_name.data(), s.sort_name.length() };
            })
         | rng::to<std::vector>();

      m_sort_combo->Append(sort_options);
   }


} // namespace ctb::app