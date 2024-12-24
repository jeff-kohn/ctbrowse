
# this call creates the target and sets compiler/linker settings and flags
create_lib_target(ctwin_lib)

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::ctwin_lib ALIAS ctwin_lib)

target_sources(ctwin_lib
   PUBLIC
      "../include/ctwin/constants.h"
      "../include/ctwin/environment.h"
   PRIVATE
      "windows_includes.h"
      "environment.cpp"
)

target_include_directories(ctwin_lib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)