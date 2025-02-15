/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "grid/GridTableEventSource.h"

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
   class WineDetailsPanel;    // details panel


   /// @brief class for the main window of the application
   ///
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
      [[nodiscard]] static MainFrame* create();


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
      CellarTrackerGrid*      m_grid{};         // grid window view
      GridOptionsPanel*       m_grid_options{}; // gird options view
      GridTableEventSourcePtr m_event_source{}; // for synchronizing events between views and the underlying table
      wxBoxSizer*             m_main_sizer{};
      wxMenuBar*              m_menu_bar{};
      wxSearchCtrl*           m_search_ctrl{};  // substring search box on the toolbar
      wxStatusBar*            m_status_bar{};
      wxToolBar*              m_tool_bar{};
      WineDetailsPanel*       m_wine_details{};

      /// @brief private ctor called by static create()
      MainFrame();

      // child window creation
      void initControls();
      void createGridWindows();
      void createMenuBar();
      void createStatusBar();
      void createToolBar();

      // message handlers
      void onMenuEditFind(wxCommandEvent& event);
      void onMenuPreferences(wxCommandEvent&);
      void onMenuSyncData(wxCommandEvent&);
      void onMenuWineList(wxCommandEvent&);
      void onSearchBtn(wxCommandEvent& event);
      void onSearchCancelBtn(wxCommandEvent& event);
      void onSearchTextEnter(wxCommandEvent& event);
      void onQuit(wxCommandEvent&);

      // implementation details
      void doSearchFilter();
      void clearSearchFilter();
      void updateStatusBarCounts();
   };


} // namespace ctb::app

