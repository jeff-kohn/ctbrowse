
# this call creates the target and sets compiler/linker settings and flags
create_lib_target(ctBrowse_lib)

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::ctBrowse_lib ALIAS ctBrowse_lib)

target_sources(ctBrowse_lib
   PUBLIC
      "../include/ctb/common_types.h"
      "../include/ctb/concepts.h"
      "../include/ctb/constants.h"
      "../include/ctb/CredentialWrapper.h"
      "../include/ctb/ctb.h"
      "../include/ctb/CtRecordImpl.h"
      "../include/ctb/DisplayColumn.h"
      "../include/ctb/Error.h"
      "../include/ctb/PropFilter.h"
      "../include/ctb/PropStringFilter.h"
      "../include/ctb/PropStringFilterMgr.h"
      "../include/ctb/SubstringFilter.h"
      "../include/ctb/TableProperty.h"
      "../include/ctb/TableSorter.h"
      "../include/ctb/table_data.h"
      "../include/ctb/table_download.h"
      "../include/ctb/utility.h"
      "../include/ctb/utility_http.h"
      "../include/ctb/utility_templates.h"
      "../include/ctb/WineListTraits.h"

   PRIVATE
      "CredentialWrapper.cpp"
      "table_download.cpp"
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
      magic_enum::magic_enum
      "$<$<PLATFORM_ID:Windows>:Credui>"
      "$<$<PLATFORM_ID:Windows>:shlwapi>"
)


# ---- End-of-file commands ----

add_folders(lib)
