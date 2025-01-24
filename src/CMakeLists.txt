

create_windows_target(ctbrowse_win)

target_sources(ctbrowse_win
   PRIVATE
     "App.cpp"
     "App.h"
     "generated/MainFrameBase.cpp"
     "generated/MainFrameBase.h"
     "generated/TableSyncDlgBase.cpp"
     "generated/TableSyncDlgBase.h"
     "grids/CellarTrackerGrid.cpp"
     "grids/CellarTrackerGrid.h"
     "grids/GridTableBase.h"
     "grids/GridTableMgr.cpp"
     "grids/GridTableMgr.h"
     "grids/GridTableWineList.h"
     "grids/GridTableWineList.cpp"
     "MainFrame.cpp"
     "MainFrame.h"
     "TableSyncDialog.cpp"
     "TableSyncDialog.h"
     "wx_helpers.h"
     "utf8.manifest"
)

find_package(wxWidgets CONFIG REQUIRED)

target_include_directories(ctbrowse_win 
   PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(ctbrowse_win
   PRIVATE
      ctbrowse::ctb_lib
      wxWidgets::wxWidgets
)

# ---- End-of-file commands ----
add_folders(WinApp)
