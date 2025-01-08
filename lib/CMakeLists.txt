
# this call creates the target and sets compiler/linker settings and flags
create_lib_target(ctwin_lib)

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::ctwin_lib ALIAS ctwin_lib)

target_sources(ctwin_lib
   PUBLIC
      "../include/ctwin/CellarTrackerDownload.h"
      "../include/ctwin/constants.h"
      "../include/ctwin/winapi_util.h"
      "../include/ctwin/Error.h"
   PRIVATE
      
      "CellarTrackerDownload.cpp"
      "CredentialWrapper.cpp"
      "../include/ctwin/CredentialWrapper.h"
      "winapi_util.cpp"
      
)

target_include_directories(ctwin_lib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Note that since this target is a 
target_link_libraries(ctwin_lib
   PUBLIC
      cpr::cpr
      magic_enum::magic_enum
      Credui
      shlwapi
)
