
# this call creates the target and sets compiler/linker settings and flags
create_lib_target(ctb_lib)

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::ctb_lib ALIAS ctb_lib)

target_sources(ctb_lib
   PUBLIC
      "../include/ctb/constants.h"
      "../include/ctb/CredentialWrapper.h"
      "../include/ctb/data/table_data.h"
      "../include/ctb/data/WineListEntry.h"
      "../include/ctb/Error.h"
      "../include/ctb/winapi_util.h"

   PRIVATE
      "CredentialWrapper.cpp"
      "data/table_download.cpp"
      "data/WineListEntry.cpp"
      "winapi_util.cpp"
)

target_include_directories(ctb_lib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Note that since this target is a 
target_link_libraries(ctb_lib
   PUBLIC
      cpr::cpr
      magic_enum::magic_enum
      Credui
      shlwapi
)


# ---- End-of-file commands ----

add_folders(lib)
