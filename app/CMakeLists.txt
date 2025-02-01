find_package(wxWidgets CONFIG REQUIRED)

set(target_name ctBrowse_app)

# vars used for version resource
set (${PROJECT_NAME}_AUTHOR "Jeff Kohn")
string(TIMESTAMP CURRENT_YEAR "%Y")
set (${PROJECT_NAME}_COPYRIGHT "Copyright © ${CURRENT_YEAR} ${${PROJECT_NAME}_AUTHOR}")
set(${PROJECT_NAME}_VERSION_RESOURCE "")

generate_product_version(
   ${PROJECT_NAME}_VERSION_RESOURCE
   NAME ${target_name}
   FILE_DESCRIPTION "${PROJECT_NAME} for Windows"
   ICON ${CMAKE_CURRENT_LIST_DIR}/resources/art/product.ico
   ORIGINAL_FILENAME ${target_name}${CMAKE_EXECUTABLE_SUFFIX}
   VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
   VERSION_MINOR ${PROJECT_VERSION_MINOR}
   VERSION_PATCH ${PROJECT_VERSION_PATCH}
   VERSION_REVISION ${PROJECT_VERSION_TWEAK}
   COMPANY_NAME ${${PROJECT_NAME}_AUTHOR}
   COMPANY_COPYRIGHT  ${${PROJECT_NAME}_COPYRIGHT}
)

create_windows_target(${target_name} ALIAS ${target_name})

# disable cmake's generated manifest because we're using one from wxWidgets
set_target_properties(${target_name} PROPERTIES LINK_FLAGS "/MANIFEST:NO")

target_include_directories(${target_name} 
   ${warning_guard}
   PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${target_name}
   PRIVATE
      ctBrowse::ctBrowse_lib
      wxWidgets::wxWidgets
)

target_sources(${target_name}
   PRIVATE
     "App.cpp"
     "App.h"
     "app_constants.h"
     "MainFrame.cpp"
     "MainFrame.h"
     "wx_helpers.h"
     "dialogs/TableSyncDialog.cpp"
     "dialogs/TableSyncDialog.h"
     "grids/CellarTrackerGrid.cpp"
     "grids/CellarTrackerGrid.h"
     "grids/GridTableBase.h"
     "grids/GridTableMgr.cpp"
     "grids/GridTableMgr.h"
     "grids/GridTableWineList.h"
     "grids/GridTableWineList.cpp"
     "panels/GridOptionsPanel.cpp"
     "panels/GridOptionsPanel.h"
     "resources/resource.rc"
     "resources/utf8.manifest"
     ${${PROJECT_NAME}_VERSION_RESOURCE}
)


# ---- End-of-file commands ----
add_folders(app)
