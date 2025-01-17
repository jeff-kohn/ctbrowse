
# this call creates the target and sets compiler/linker settings and flags
create_lib_target(cts_lib)

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::cts_lib ALIAS cts_lib)

target_sources(cts_lib
   PUBLIC
      "../include/cts/CellarTrackerDownload.h"
      "../include/cts/constants.h"
      "../include/cts/CredentialWrapper.h"
      "../include/cts/Error.h"
      "../include/cts/winapi_util.h"
      "../include/cts/data/WineList.h"

   PRIVATE
      "data/WineList.cpp"
      "CellarTrackerDownload.cpp"
      "CredentialWrapper.cpp"
      "winapi_util.cpp"
)

target_include_directories(cts_lib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Note that since this target is a 
target_link_libraries(cts_lib
   PUBLIC
      cpr::cpr
      magic_enum::magic_enum
      Credui
      shlwapi
)


# ---- End-of-file commands ----

add_folders(lib)
