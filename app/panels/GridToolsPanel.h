#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

namespace ctb::app
{
   class CellarTrackerGrid;


   /// @brief panel class that provides UI for searching, sorting, and filtering a grid
   class GridToolsPanel final : public wxPanel
   {
   public:
      GridToolsPanel() = default;
      GridToolsPanel(wxWindow* parent, CellarTrackerGrid* grid);

      bool Create(wxWindow* parent);

   private:
      CellarTrackerGrid* m_grid{};
      wxTextCtrl*        m_search_ctrl{};
      wxString           m_search_value{};
      wxTimer            m_timer{};
      
      void onClearSearchClicked(wxCommandEvent& event);
      void onSearchTextChanged(wxCommandEvent& event);
      void onSearchTimer(wxTimerEvent& event);
      void doFilter();
      void createImpl();
   };

} // namespace ctb::app