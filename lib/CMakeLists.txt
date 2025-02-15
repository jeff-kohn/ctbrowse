
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
      "../include/ctb/Error.h"
      "../include/ctb/winapi_util.h"

      "../include/ctb/data/DisplayColumn.h"
      "../include/ctb/data/PropertyFilterMgr.h"
      "../include/ctb/data/PropertyFilterString.h"
      "../include/ctb/data/SubstringFilter.h"
      "../include/ctb/data/TableProperty.h"
      "../include/ctb/data/TableSorter.h"
      "../include/ctb/data/table_data.h"
      "../include/ctb/data/table_download.h"
      "../include/ctb/data/WineListEntry.h"

   PRIVATE
      "CredentialWrapper.cpp"
      "data/table_download.cpp"
      "data/WineListEntry.cpp"
      "winapi_util.cpp"
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
      magic_enum::magic_enum
      Credui
      shlwapi
)


# ---- End-of-file commands ----

add_folders(lib)
