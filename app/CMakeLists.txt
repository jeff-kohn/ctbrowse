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
)

target_sources(${TARGET_NAME}
   PRIVATE
     "App.cpp"
     "App.h"
     "app_constants.h"
     "LabelImageCache.cpp"
     "LabelImageCache.h"
     "MainFrame.cpp"
     "MainFrame.h"
     "CtCredentialManager.cpp"
     "CtCredentialManager.h"
     "LoginEvent.h"
     "wx_helpers.h"
     "dialogs/CredentialDialog.cpp"
     "dialogs/CredentialDialog.h"
     "dialogs/TableSyncDialog.cpp"
     "dialogs/TableSyncDialog.h"
     "model/CtDataViewModel.h"
     "views/DatasetListView.cpp"
     "views/DatasetListView.h"
     "views/DatasetMultiView.cpp"
     "views/DatasetMultiView.h"
     "views/DatasetOptionsPanel.cpp"
     "views/DatasetOptionsPanel.h"
     "views/DetailsPanel.cpp"
     "views/DetailsPanel.h"
     "resources/utf-8.manifest"
     "resources/resource.rc"
     ${${PROJECT_NAME}_VERSION_RESOURCE}
)

add_manifest(${TARGET_NAME} ${MANIFEST_FILE} )

# ---- End-of-file commands ----
add_folders(app)
