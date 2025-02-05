#include "MainFrame.h"
#include "App.h"
#include "panels/GridOptionsPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

#include <memory>

namespace ctb::app
{
   
   GridOptionsPanel::GridOptionsPanel(GridTableEventSourcePtr source) : m_sink{ this, source }
   {}


   [[nodiscard]] GridOptionsPanel* GridOptionsPanel::create(wxWindow* parent, GridTableEventSourcePtr source)
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_NULLPTR_ARG };
      }
      if (!parent)
      {
         assert("parent parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_NULLPTR_ARG };
      }

      std::unique_ptr<GridOptionsPanel> wnd{ new GridOptionsPanel{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   void GridOptionsPanel::initControls()
   {
      // panel shouldn't grow infinitely
      SetMaxSize(ConvertDialogToPixels(wxSize{ 125, constants::UNSPECIFIED }));

      // defines the rows of controls in our panel
      auto top_sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
      auto default_border = wxSizerFlags::GetDefaultBorder();

      // combo for selecting the sort option
      m_sort_combo = new wxChoice(this, wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_idx));

      // label for the sort combo
      auto* lbl_sort_by = new wxStaticText{ this, wxID_ANY, constants::LBL_SORT_BY };

      top_sizer->Add(lbl_sort_by, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));
      top_sizer->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, default_border));

      SetSizerAndFit(top_sizer.release());
   }


   void GridOptionsPanel::populateSortOptions(IGridTable* grid_table)
   {
      using SortName = IGridTable::SortOptionName;
      auto sort_options = vws::all(grid_table->availableSortOptions()) 
         | vws::transform([](const SortName& s) -> wxString 
            {  
               return wxString{s.sort_name.data(), s.sort_name.length() };
            })
         | rng::to<std::vector>();

      m_sort_combo->Clear();
      m_sort_combo->Append(sort_options);
   }


   void GridOptionsPanel::notify(GridTableEvent event, IGridTable* grid_table)
   {
      switch (event)
      {
         case GridTableEvent::TableInitialize:
            populateSortOptions(grid_table);
            break;

         case GridTableEvent::Sort:
            break;
         case GridTableEvent::Filter:
            break;
         case GridTableEvent::SubStringFilter:
            break;
         case GridTableEvent::RowSelected:
            break;
         default:
            break;
      }   
   }

} // namespace ctb::app