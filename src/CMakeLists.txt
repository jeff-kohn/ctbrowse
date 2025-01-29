
create_windows_target(ctbrowse_win)

target_sources(ctbrowse_win
   PRIVATE
     "App.cpp"
     "App.h"
     "dialogs/TableSyncDialog.cpp"
     "dialogs/TableSyncDialog.h"
     "grids/CellarTrackerGrid.cpp"
     "grids/CellarTrackerGrid.h"
     "grids/GridTableBase.h"
     "grids/GridTableMgr.cpp"
     "grids/GridTableMgr.h"
     "grids/GridTableWineList.h"
     "grids/GridTableWineList.cpp"
     "MainFrame.cpp"
     "MainFrame.h"
     "panels/GridToolsPanel.cpp"
     "panels/GridToolsPanel.h"
     "wx_helpers.h"
     "utf8.manifest"
     ${CMAKE_CURRENT_BINARY_DIR}/images_resource.cpp
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

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/images_resource.cpp
    COMMAND ${WXRC_PATH} -c -n initImageResource -o ${CMAKE_CURRENT_BINARY_DIR}/images_resource.cpp ${CMAKE_CURRENT_SOURCE_DIR}/art/images.xrc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/art/images.xrc
)

# ---- End-of-file commands ----
add_folders(WinApp)
