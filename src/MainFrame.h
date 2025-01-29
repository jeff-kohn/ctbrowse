/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "grids/CellarTrackerGrid.h"
#include "panels/GridToolsPanel.h"

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/grid.h>
#include <wx/menu.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statusbr.h>
#include <wx/toolbar.h>

#include <format>


namespace ctb
{
   class MainFrame final : public wxFrame
   {
   public:
      static inline constexpr int STATUS_BAR_PANE_STATUS = 0;
      static inline constexpr int STATUS_BAR_PANE_FILTERED_ROWS = 1;
      static inline constexpr int STATUS_BAR_PANE_TOTAL_ROWS = 2;

      /// @brief default constructor for 2-phase initialization, must call Create(parent)
      MainFrame();  


      /// @brief constructor for immediate initialization, calls Create(parent) for you
      MainFrame(wxWindow* parent);


      /// @brief create the window object
      ///
      /// this should only be directly called if this object was default-constructed
      bool Create(wxWindow* parent);


      /// @brief set status bar text using std::format() syntax
      template <typename... Args>
      constexpr void setStatusText(std::format_string<Args...> fmt_str, Args&&... args)
      {
         SetStatusText(std::format(fmt_str, std::forward<Args>(args)...));
      }


      /// @brief set status bar text for a specified pane using std::format() syntax
      ///
      /// pane-index is zero-based
      template <typename... Args>
      constexpr void setStatusText(int pane_index, std::format_string<Args...> fmt_str, Args&&... args)
      {
         assert(pane_index <= STATUS_BAR_PANE_FILTERED_ROWS);
         SetStatusText(std::format(fmt_str, std::forward<Args>(args)...), pane_index);
      }


      /// @brief retrieves a pointer to the active grid.
      /// @return 
      CellarTrackerGrid* getGrid() { return m_grid; }

   private:
      CellarTrackerGrid*   m_grid{};
      wxBoxSizer*          m_main_sizer{};
      wxMenuBar*           m_menu_bar{};
      GridToolsPanel*      m_grid_tools_panel{};
      wxSearchCtrl*        m_search_ctrl{};
      wxSplitterWindow*    m_splitter{};
      wxStatusBar*         m_status_bar{};
      wxToolBar*           m_tool_bar{};

      void createGrid();
      void createMenuBar();
      void createStatusBar();
      void createToolBar();

      void onMenuEditFind(wxCommandEvent& event);
      void onMenuPreferences(wxCommandEvent&);
      void onMenuSyncData(wxCommandEvent&);
      void onMenuWineList(wxCommandEvent&);
      void onSearchBtn(wxCommandEvent& event);
      void onSearchCancelBtn(wxCommandEvent& event);
      void onSearchTextEnter(wxCommandEvent& event);
      void onQuit(wxCommandEvent&);

      void doSearchFilter();
      void clearSearchFilter();
      void updateStatusBarCounts();
      void createImpl();
   };


} // namespace ctb

