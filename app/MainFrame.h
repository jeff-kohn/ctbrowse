/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include <ctb/model/DatasetEventSource.h>
#include <ctb/model/ScopedEventSink.h>

#include <wx/event.h>
#include <wx/frame.h>


/// forward declare wx classes to avoid header pollution.
class wxBoxSizer;
class wxMenuBar;
class wxSearchCtrl;
class wxSplitterWindow;
class wxStatusBar;
class wxToolBar;


namespace ctb::app
{
   class DatasetMultiView;
   class LabelImageCache;     // used for retrieving label images


   // we don't use enum class because then every time we need to pass an ID to wxObject,
   // we'd have to cast or use std::to_underlying and that's just an ugly waste of time 
   // with no benefit for this use-case.
   enum CmdId : uint16_t
   {
      CMD_FILE_DOWNLOAD_DATA = wxID_HIGHEST,
      CMD_FILE_SETTINGS,
      CMD_DATA_WINE_LIST,
      CMD_DATA_PENDING_WINE,
      CMD_WINE_ONLINE_DETAILS,
      CMD_WINE_ONLINE_VINTAGES,
      CMD_WINE_ONLINE_PRODUCER,
      CMD_WINE_ONLINE_ACCEPT_WINE,
      CMD_VIEW_AUTOLAYOUT_COLS
   };

   /// @brief class for the main window of the application
   ///
   class MainFrame final : public wxFrame, public IDatasetEventSink
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

      /// @brief set status bar text using format() syntax
      ///
      template <typename... Args>
      constexpr void setStatusText(ctb::format_string<Args...> fmt_str, Args&&... args)
      {
         SetStatusText(ctb::format(fmt_str, std::forward<Args>(args)...));
      }

      /// @brief set status bar text for a specified pane using format() syntax
      ///
      /// pane-index is zero-based
      ///
      template <typename... Args>
      constexpr void setStatusText(int pane_index, ctb::format_string<Args...> fmt_str, Args&&... args)
      {
         assert(pane_index <= STATUS_BAR_PANE_FILTERED_ROWS);
         SetStatusText(ctb::format(fmt_str, std::forward<Args>(args)...), pane_index);
      }

   private:
      DatasetMultiView*       m_view{};
      DatasetEventSourcePtr   m_event_source{}; // for synchronizing events between views and the underlying table
      wxMenuBar*              m_menu_bar{};
      wxSearchCtrl*           m_search_ctrl{};  // substring search box on the toolbar
      ScopedEventSink         m_sink;           // so we can also handle events from our source
      wxStatusBar*            m_status_bar{};
      wxToolBar*              m_tool_bar{};

      // we use a shared_ptr because we want to share the object with child window(s)
      std::shared_ptr<LabelImageCache> m_label_cache{};

      /// @brief private ctor called by static create()
      MainFrame();

      // child window/control creation
      void initControls();
      void createMenuBar();
      void createStatusBar();
      void createToolBar();

      // message handlers
      void onMenuEditFind(wxCommandEvent& event);
      void onMenuPreferences(wxCommandEvent&);
      void onMenuSyncData(wxCommandEvent&);
      void onMenuDataTable(wxCommandEvent&);
      void onSearchBtn(wxCommandEvent& event);
      void onMenuViewResizeGrid(wxCommandEvent&);
      void onSearchCancelBtn(wxCommandEvent& event);
      void onSearchTextEnter(wxCommandEvent& event);
      void onSearchKeyDown(wxKeyEvent& event);
      void onQuit(wxCommandEvent&);

      // implementation details
      void doSearchFilter();
      void clearSearchFilter();
      void updateStatusBarCounts();

      // Inherited via IDatasetEventSink
      void notify(DatasetEvent event) override;
   };


} // namespace ctb::app

