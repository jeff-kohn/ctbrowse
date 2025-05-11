
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
      "../include/ctb/table_data.h"
      "../include/ctb/table_download.h"
      "../include/ctb/utility.h"
      "../include/ctb/utility_chrono.h"
      "../include/ctb/utility_http.h"
      "../include/ctb/utility_templates.h"
      
      "../include/ctb/interfaces/DatasetEvent.h"
      "../include/ctb/interfaces/IDataset.h"
      "../include/ctb/interfaces/IDatasetEventSink.h"
      "../include/ctb/interfaces/IDatasetEventSource.h"
            
      "../include/ctb/model/CtDataModel.h"
      "../include/ctb/model/CtDisplayColumn.h"
      "../include/ctb/model/DatasetEventSource.h"
      "../include/ctb/model/DatasetLoader.h"
      "../include/ctb/model/ScopedEventSink.h"
      
      "../include/ctb/tables/CtDataTable.h"
      "../include/ctb/tables/CtProperty.h"
      "../include/ctb/tables/PendingWineTable.h"
      "../include/ctb/tables/TableRecord.h"
      "../include/ctb/tables/WineListTable.h"

      "../include/ctb/tables/detail/MultiMatchPropertyFilter.h"
      "../include/ctb/tables/detail/MultiMatchPropertyFilterMgr.h"
      "../include/ctb/tables/detail/PropertyFilter.h"
      "../include/ctb/tables/detail/SortConfig.h"
      "../include/ctb/tables/detail/SubstringFilter.h"
      "../include/ctb/tables/detail/TableProperty.h"
      "../include/ctb/tables/detail/TableSorter.h"

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
