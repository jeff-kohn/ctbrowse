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
      DatasetEventSourcePtr   m_event_source{}; // for synchronizing events between views and the underlying dataset
      wxMenuBar*              m_menu_bar{};
      wxSearchCtrl*           m_search_ctrl{};  // substring search box on the toolbar
      ScopedEventSink         m_sink;           // so we can also handle events from our source
      wxStatusBar*            m_status_bar{};
      wxToolBar*              m_tool_bar{};
      int                     m_selected_row{ -1 }; // whether or not a row is selected in the dataset view, for update-UI handlers

      // we use a shared_ptr because we want to share the object with child window(s)
      std::shared_ptr<LabelImageCache> m_label_cache{};

      /// @brief private ctor called by static create()
      MainFrame();

      // child window/control creation
      void initControls();
      void createMenuBar();
      void createStatusBar();
      void createToolBar();

      // File menu handlers
      void onMenuFilePreferences(wxCommandEvent&);
      void onMenuFileSyncData(wxCommandEvent&);
      void onMenuFileQuit(wxCommandEvent&);

      // Edit menu handlers
      void onMenuEditFind(wxCommandEvent& event);
      
      // Collection menu handlers
      void onMenuCollection(wxCommandEvent&);

      // Online menu events
      void onMenuWineAddTastingNote(wxCommandEvent&);
      void onMenuWineAddToCellar(wxCommandEvent&);
      void onMenuWineAcceptDelivery(wxCommandEvent&);
      void onMenuWineEditOrder(wxCommandEvent&);
      void onMenuWineDrinkRemove(wxCommandEvent&);
      void onMenuWineOnlineDetails(wxCommandEvent&);
      void onMenuWineOnlineVintages(wxCommandEvent&);

      // UI update handlers for wine online commands
      void onMenuWineAcceptDeliveryUI(wxUpdateUIEvent&);  
      void onMenuWineAddToCellarUI(wxUpdateUIEvent&);  
      void onMenuWineDrinkRemoveUI(wxUpdateUIEvent&);
      void onMenuWineOnlineUI(wxUpdateUIEvent&);

      // Toolbar event handlers
      void onToolbarSearchBtn(wxCommandEvent& event);
      void onToolbarSearchCancelBtn(wxCommandEvent& event);
      void onToolbarSearchKeyDown(wxKeyEvent& event);
      void onToolbarSearchTextEnter(wxCommandEvent& event);

      // implementation details
      void clearSearchFilter();
      void doSearchFilter();
      auto getDataset(bool throw_on_null = true) -> DatasetPtr;
      void updateStatusBarCounts();

      // Inherited via IDatasetEventSink
      void notify(DatasetEvent event) override;
   };


} // namespace ctb::app

