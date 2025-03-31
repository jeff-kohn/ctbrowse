find_package(wxWidgets CONFIG REQUIRED)

set(TARGET_NAME ctBrowse_app)

# vars used for later statements
set (${PROJECT_NAME}_AUTHOR "Jeff Kohn")
string(TIMESTAMP CURRENT_YEAR "%Y")
set (${PROJECT_NAME}_COPYRIGHT "Copyright © ${CURRENT_YEAR} ${${PROJECT_NAME}_AUTHOR}")
set(${PROJECT_NAME}_VERSION_RESOURCE "")
set(MANIFEST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/utf-8.manifest" )


generate_product_version(
   ${PROJECT_NAME}_VERSION_RESOURCE
   NAME ${TARGET_NAME}
   FILE_DESCRIPTION "${PROJECT_NAME} for Windows"
   ICON ${CMAKE_CURRENT_LIST_DIR}/resources/art/product.ico
   ORIGINAL_FILENAME ${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}
   VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
   VERSION_MINOR ${PROJECT_VERSION_MINOR}
   VERSION_PATCH ${PROJECT_VERSION_PATCH}
   VERSION_REVISION ${PROJECT_VERSION_TWEAK}
   COMPANY_NAME ${${PROJECT_NAME}_AUTHOR}
   COMPANY_COPYRIGHT  ${${PROJECT_NAME}_COPYRIGHT}
)

create_windows_target(${TARGET_NAME} ALIAS ${TARGET_NAME})

# disable cmake's generated manifest because we're using one from wxWidgets
set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/MANIFEST:NO")

target_include_directories(${TARGET_NAME} 
   ${warning_guard}
   PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${TARGET_NAME}
   PRIVATE
      ctBrowse::ctBrowse_lib
      wxWidgets::wxWidgets
      libcoro
      spdlog::spdlog
)

target_sources(${TARGET_NAME}
   PRIVATE
     "App.cpp"
     "App.h"
     "app_constants.h"
     "LabelImageCache.h"
     "LabelImageCache.cpp"
     "log.h"
     "log.cpp"
     "MainFrame.cpp"
     "MainFrame.h"
     "tasks.h"
     "tasks.cpp"
     "wx_helpers.h"
     "dialogs/TableSyncDialog.cpp"
     "dialogs/TableSyncDialog.h"
     "interfaces/GridTable.h"
     "interfaces/GridTableEvent.h"
     "grid/GridTableFilter.cpp"
     "grid/GridTableFilter.h"
     "grid/CellarTrackerGrid.cpp"
     "grid/CellarTrackerGrid.h"
     "grid/GridTableLoader.cpp"
     "grid/GridTableLoader.h"
     "grid/GridTableEventSource.cpp"
     "grid/GridTableEventSource.h"
     "grid/GridTableWineList.cpp"
     "grid/GridTableWineList.h"
     "grid/ScopedEventSink.h"
     "panels/GridOptionsPanel.cpp"
     "panels/GridOptionsPanel.h"
     "panels/WineDetailsPanel.cpp"
     "panels/WineDetailsPanel.h"
     "resources/utf-8.manifest"
     "resources/resource.rc"
     ${${PROJECT_NAME}_VERSION_RESOURCE}
)

add_manifest(${TARGET_NAME} ${MANIFEST_FILE} )

# ---- End-of-file commands ----
add_folders(app)
