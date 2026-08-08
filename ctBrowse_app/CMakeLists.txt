find_package(wxWidgets CONFIG REQUIRED)

set(TARGET_NAME ctBrowse_app)

# vars used for later statements
string(TIMESTAMP CURRENT_YEAR "%Y")
set(${PROJECT_NAME}_COPYRIGHT "Copyright (c) ${CURRENT_YEAR} ${${PROJECT_NAME}_AUTHOR}")
set(${PROJECT_NAME}_AUTHOR "Jeff Kohn")
set(${PROJECT_NAME}_VERSION_RESOURCE "")

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
     "CtCredentialManager.cpp"
     "CtCredentialManager.h"
     "MainFrame.cpp"
     "MainFrame.h"
     "wx_helpers.h"

     "controls/CheckBoxFilterCtrl.cpp"
     "controls/CheckBoxFilterCtrl.h"
     "controls/DrinkWindowCtrl.h"
     "controls/ElasticMultiLineTextCtrl.cpp"
     "controls/ElasticMultiLineTextCtrl.h"
     "controls/ElasticPropertyValueBase.cpp"
     "controls/ElasticPropertyValueBase.h"
     "controls/LabelImageCtrl.cpp"
     "controls/LabelImageCtrl.h"
     "controls/MultiValueFilterTreeCtrl.cpp"
     "controls/MultiValueFilterTreeCtrl.h"
     "controls/PropertyValueCtrl.h"
     "controls/ProReviewsCacheCtrl.h"
     "controls/SpinDoubleFilterCtrl.cpp"
     "controls/SpinDoubleFilterCtrl.h"

     "dialogs/CredentialDialog.cpp"
     "dialogs/CredentialDialog.h"
     "dialogs/TableSyncDialog.cpp"
     "dialogs/TableSyncDialog.h"

     "model/CtDataViewModel.cpp"
     "model/CtDataViewModel.h"
     "model/DatasetDefaultOptions.cpp"
     "model/DatasetDefaultOptions.h"
     "model/DatasetWindow.h"

     "panels/SortOptionsPanel.cpp"
     "panels/SortOptionsPanel.h"
     "panels/WineDetailBasePanel.cpp"
     "panels/WineDetailBasePanel.h"
     "panels/WineDetailBottleInfoPanel.h"
     "panels/WineDetailMainPanel.h"
     "panels/WineDetailPanelRow.h"
     "panels/WineDetailPendingPanel.h"
     "panels/WineDetailPrivateNotesPanel.h"
     "panels/WineDetailScorePanel.h"
     "panels/WineDetailTagsPanel.h"
     "panels/WineDetailTastingPanel.cpp"
     "panels/WineDetailTastingPanel.h"
     "panels/WineDetailValuePanel.h"

     "views/DetailsViewBase.cpp"
     "views/DetailsViewBase.h"
     "views/DetailsViewBottleInventory.h"
     "views/DetailsViewConsumedWine.h"
     "views/DetailsViewMyCellar.h"
     "views/DetailsViewPending.h"
     "views/DetailsViewPrivateNotes.h"
     "views/DetailsViewPurchasedWine.h"
     "views/DetailsViewReadyToDrink.h"
     "views/DetailsViewTaggedWine.h"
     "views/DetailsViewTastingNotes.h"

     
     "views/DatasetListView.cpp"
     "views/DatasetListView.h"
     "views/DatasetMultiView.cpp"
     "views/DatasetMultiView.h"
     "views/DatasetOptionsView.cpp"
     "views/DatasetOptionsView.h"
     "views/DetailsViewMyCellar.h"
     "resources/ctBrowse.manifest"
     "resources/resource.rc"
     ${${PROJECT_NAME}_VERSION_RESOURCE}
 )


if (CTB_USE_PRECOMPILED_HEADERS)
   target_precompile_headers(${TARGET_NAME}
      PRIVATE
         <algorithm>
         <array>
         <boost/algorithm/string/predicate.hpp>
         <boost/range/as_literal.hpp>
         <boost/unordered/unordered_flat_map.hpp>
         <chrono>
         <concepts>
         <enchantum/enchantum.hpp>
         <enchantum/fmt_format.hpp>
         <exception>
         <expected>
         <external/csv.hpp>
         <filesystem>
         <fmt/chrono.h>
         <fmt/format.h>
         <frozen/map.h>
         <map>
         <memory>
         <optional>
         <ranges>
         <set>
         <span>
         <string_view>
         <string>
         <unordered_set>
         <variant>
         <vector>
         <wx/app.h>
         <wx/button.h>
         <wx/checklst.h>
         <wx/config.h>
         <wx/event.h>
         <wx/fileconf.h>
         <wx/frame.h>
         <wx/gdicmn.h>
         <wx/panel.h>
         <wx/sizer.h>
         <wx/stattext.h>
         <wx/textctrl.h>
         <wx/valgen.h>
         <wx/window.h>
         <wx/windowptr.h>
         <wx/wupdlock.h>
         <wx/xrc/xmlres.h>
         <ctb/ctb.h>
   )
   message(STATUS "precompiled header support enabled for ${TARGET_NAME}")
endif()

# ---- End-of-file commands ----
# add_folders(app)
