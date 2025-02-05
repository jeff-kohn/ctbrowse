/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "grid/GridTableSource.h"

#include <wx/event.h>
#include <wx/frame.h>

#include <format>


/// forward declare wxWidget classes to avoid header pollution.
class wxBoxSizer;
class wxMenuBar;
class wxSearchCtrl;
class wxSplitterWindow;
class wxStatusBar;
class wxToolBar;


namespace ctb::app
{
   class CellarTrackerGrid;   // the grid window
   class GridOptionsPanel;    // the options panel


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

   private:
      CellarTrackerGrid*      m_grid{};
      wxBoxSizer*             m_main_sizer{};
      wxMenuBar*              m_menu_bar{};
      GridOptionsPanel*       m_grid_options{};
      wxSearchCtrl*           m_search_ctrl{};
      wxSplitterWindow*       m_splitter{};
      wxStatusBar*            m_status_bar{};
      wxToolBar*              m_tool_bar{};
      GridTableEventSourcePtr m_event_source{};

      void createGridWindows();
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
   };


} // namespace ctb::app

