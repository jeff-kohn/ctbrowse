
# this call creates the target and sets compiler/linker settings and flags
create_lib_target(ctBrowse_lib)

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::ctBrowse_lib ALIAS ctBrowse_lib)

target_sources(ctBrowse_lib
   PUBLIC
      "../include/ctb/ctb.h"
      "../include/ctb/common_types.h"
      "../include/ctb/concepts.h"
      "../include/ctb/constants.h"
      "../include/ctb/CredentialManager.h"
      "../include/ctb/CredentialWrapper.h"
      "../include/ctb/Error.h"
      "../include/ctb/log.h"
      "../include/ctb/utility.h"
      "../include/ctb/utility_chrono.h"
      "../include/ctb/utility_http.h"
      "../include/ctb/utility_templates.h"
      
      "../include/ctb/model/CtDataModel.h"
      "../include/ctb/model/CtStringFilter.h"
      "../include/ctb/model/DatasetBase.h"
      "../include/ctb/model/DatasetEvent.h"
      "../include/ctb/model/DatasetEventSource.h"
      "../include/ctb/model/DatasetLoader.h"
      "../include/ctb/model/DisplayColumn.h"
      "../include/ctb/model/ScopedEventSink.h"
      
      "../include/ctb/tables/CtDataTable.h"
      "../include/ctb/tables/PendingWineTable.h"
      "../include/ctb/tables/WineListTable.h"
      "../include/ctb/tables/PropFilter.h"
      "../include/ctb/tables/PropStringFilter.h"
      "../include/ctb/tables/PropStringFilterMgr.h"
      "../include/ctb/tables/SubstringFilter.h"
      "../include/ctb/tables/TableProperty.h"
      "../include/ctb/tables/TableRecord.h"
      "../include/ctb/tables/TableSorter.h"
      "../include/ctb/tables/table_data.h"
      "../include/ctb/tables/table_download.h"

      "../include/ctb/tasks/PollingTask.h"
      "../include/ctb/tasks/tasks.h"

   PRIVATE
      "CredentialWrapper.cpp"
      "DatasetEventSource.cpp"
      "DatasetLoader.cpp"
      "log.cpp"
      "table_download.cpp"
      "tasks.cpp"
      "utility.cpp"
      "utility_http.cpp"
)

target_include_directories(ctBrowse_lib
   ${warning_guard}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(ctBrowse_lib
   SYSTEM
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3rdParty>
)

# Note that since this target is a 
target_link_libraries(ctBrowse_lib
   PUBLIC
      cpr::cpr
      fmt::fmt
      HtmlParser
      magic_enum::magic_enum
      spdlog::spdlog
      "$<$<PLATFORM_ID:Windows>:Credui>"
      "$<$<PLATFORM_ID:Windows>:shlwapi>"
)


# ---- End-of-file commands ----

add_folders(lib)
