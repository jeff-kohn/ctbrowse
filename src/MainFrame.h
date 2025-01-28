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

      /// @brief retrieves a pointer to the active grid.
      /// @return 
      CellarTrackerGrid* getGrid() { return m_grid; }

   private:
      CellarTrackerGrid*   m_grid{};
      wxBoxSizer*          m_main_sizer{};
      wxMenuBar*           m_menu_bar{};
      GridToolsPanel*      m_grid_tools_panel{};
      wxSplitterWindow*    m_splitter{};
      wxStatusBar*         m_status_bar{};
      wxToolBar*           m_tool_bar{};

      void onMenuPreferences(wxCommandEvent&);
      void onMenuSyncData(wxCommandEvent&);
      void onMenuWineList(wxCommandEvent&);
      void onQuit(wxCommandEvent&);
      void createImpl();
   };


} // namespace ctb

