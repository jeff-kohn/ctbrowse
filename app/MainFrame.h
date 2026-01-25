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
#include <wx/menu.h>
#include <wx/frame.h>

#include <memory>

/// forward declare wx classes to avoid header pollution.
class wxBoxSizer;
class wxMenu;
class wxMenuBar;
class wxSearchCtrl;
class wxSplitterWindow;
class wxStatusBar;
class wxToolBar;


namespace ctb::app
{
   class DatasetMultiView;    // main child window, contains list view, options panel and details panel


   /// @brief class for the main window of the application
   ///
   class MainFrame final : public wxFrame, public IDatasetEventSink
   {
   public:
      static inline constexpr int STATUS_BAR_PANE_STATUS  = 0;
      static inline constexpr int STATUS_BAR_PANE_CENTER  = 1;
      static inline constexpr int STATUS_BAR_PANE_SUMMARY = 2;

      /// @brief static factor method to create an initialize an instance of the MainFrame class
      /// 
      /// throws a ctb::Error if the window can't be created; otherwise returns a non-owning pointer 
      /// to the window (top-level window so it will manage its own lifetime). 
      /// 
      [[nodiscard]] static MainFrame* create();

      /// @brief set status bar text using format() syntax
      template <typename... Args>
      constexpr void setStatusText(ctb::format_string<Args...> fmt_str, Args&&... args)
      {
         SetStatusText(ctb::format(fmt_str, std::forward<Args>(args)...));
      }

      /// @brief Type alias for a wxMenu smart ptr 
      using wxMenuPtr = std::unique_ptr<wxMenu>;

      /// @brief Get a popup menu with commands relevant to the selected wine
      auto getWinePopupMenu() const -> wxMenuPtr;

   private:
      static inline constexpr int ROW_NONE = -1;

      DatasetMultiView*     m_view{};         // non-owning ptr to main child window
      DatasetEventSourcePtr m_event_source{}; // for synchronizing events between views and the underlying dataset
      wxMenuBar*            m_menu_bar{};     // non-owning ptr to main menubar
      wxSearchCtrl*         m_search_ctrl{};  // non-owning ptr to substring search box on the toolbar
      ScopedEventSink       m_sink;           // so we can also handle events from our source
      wxStatusBar*          m_status_bar{};   // non-owning ptr to statusbar ctrl
      wxToolBar*            m_tool_bar{};     // non-owning ptr to toolbar ctrl
      int                   m_selected_row{ ROW_NONE }; // whether or not a row is selected in the dataset view, for update-UI handlers. -1 means no selection

      /// @brief private ctor called by static create()
      MainFrame();

      // child window/control creation
      void initControls();
      void createMenuBar();
      void createToolBar();

      // File menu handlers
      void onMenuFileOpen(wxCommandEvent&); 
      void onMenuFileSave(wxCommandEvent&);
      void onMenuFilePreferences(wxCommandEvent&);
      void onMenuFileSyncData(wxCommandEvent&);
      void onMenuFileQuit(wxCommandEvent&);

      // Edit menu handlers
      void onMenuEditFind(wxCommandEvent& event);
      void onMenuEditRefresh(wxCommandEvent& event);
      void onMenuEditRefreshUpdateUI(wxUpdateUIEvent& event);
      void onMenuEditClearFilters(wxCommandEvent& event);
      void onMenuEditClearFiltersUpdateUI(wxUpdateUIEvent& event);
      
      // Collection menu handlers
      void onMenuCollection(wxCommandEvent&);

      // Online menu events 
      void onMenuOnlineWineDetails(wxCommandEvent&);
      void onMenuOnlineSearchVintages(wxCommandEvent&);
      void onMenuOnlineDrinkWindow(wxCommandEvent&);
      void onMenuOnlineAddToCellar(wxCommandEvent&);
      void onMenuOnlineAddTastingNote(wxCommandEvent&);
      void onMenuOnlineAcceptDelivery(wxCommandEvent&);
      void onMenuOnlineEditOrder(wxCommandEvent&);
      void onMenuOnlineDrinkRemove(wxCommandEvent&);

      // UI update handlers for wine online commands
      void onMenuOnlineAcceptDeliveryUI(wxUpdateUIEvent&);  
      void onMenuOnlineAddToCellarUI(wxUpdateUIEvent&);  
      void onMenuOnlineDrinkRemoveUI(wxUpdateUIEvent&);
      void onMenuOnlineWineSelectionUI(wxUpdateUIEvent&);

      // Toolbar event handlers
      void onToolbarSearchBtn(wxCommandEvent& event);
      void onToolbarSearchCancelBtn(wxCommandEvent& event);
      void onToolbarSearchKeyDown(wxKeyEvent& event);
      void onToolbarSearchTextEnter(wxCommandEvent& event);

      // implementation details
      void clearSearchFilter();
      void doSearchFilter();
      auto getDataset(bool throw_on_null = true) -> DatasetPtr;
      void setDataset(const DatasetPtr& dataset);
      void updateStatusBarCounts();

      // Inherited via IDatasetEventSink
      void notify(DatasetEvent event) override;
   };


} // namespace ctb::app

