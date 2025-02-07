/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
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


      /// @brief static factor method to create an initialize an instance of the MainFrame class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (top-level window so it will manage its own lifetime). 
      /// 
      static [[nodiscard]] MainFrame* create();


      /// @brief set status bar text using std::format() syntax
      ///
      template <typename... Args>
      constexpr void setStatusText(std::format_string<Args...> fmt_str, Args&&... args)
      {
         SetStatusText(std::format(fmt_str, std::forward<Args>(args)...));
      }


      /// @brief set status bar text for a specified pane using std::format() syntax
      ///
      /// pane-index is zero-based
      ///
      template <typename... Args>
      constexpr void setStatusText(int pane_index, std::format_string<Args...> fmt_str, Args&&... args)
      {
         assert(pane_index <= STATUS_BAR_PANE_FILTERED_ROWS);
         SetStatusText(std::format(fmt_str, std::forward<Args>(args)...), pane_index);
      }


   private:
      CellarTrackerGrid*      m_grid{};
      GridOptionsPanel*       m_grid_options{};
      GridTableEventSourcePtr m_event_source{};
      wxBoxSizer*             m_main_sizer{};
      wxMenuBar*              m_menu_bar{};
      wxSearchCtrl*           m_search_ctrl{};
      wxSplitterWindow*       m_splitter{};
      wxStatusBar*            m_status_bar{};
      wxToolBar*              m_tool_bar{};

      /// @brief private ctor called by static create()
      MainFrame();

      void initControls();
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

