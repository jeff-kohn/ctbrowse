#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>

namespace ctb
{

   /// @brief panel class that provides UI for searching, sorting, and filtering a grid
   class GridToolsPanel final : public wxPanel
   {
   public:
      GridToolsPanel() = default;
      GridToolsPanel(wxWindow* parent_ptr);

      bool Create(wxWindow* parent_ptr);

   private:
      wxTextCtrl* m_search_ctrl{};
      wxString    m_search_value{};
      
      void onClearSearchClicked(wxCommandEvent& event);
      void onSearchTextChanged(wxCommandEvent& event);
      void createImpl();
   };

} // namespace ctb