cmake_minimum_required (VERSION 3.30)

include(cmake/prelude.cmake)

project(
    ct_search
    VERSION 0.1.0
    DESCRIPTION "CellarTracker Search"
    LANGUAGES CXX
)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(cmake/add_target.cmake)
include(cmake/folders.cmake)
include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

 #export compile_commands.json for tools like clang-tidy and cppcheck
 set (CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") 

   # Only do these if this is the main project, and not if it is included through add_subdirectory
if (PROJECT_IS_TOP_LEVEL)
   # Let's nicely support folders in IDEs
   set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   include(cmake/folders.cmake)
endif()

find_path(P_RANAV_CSV2_INCLUDE_DIRS "csv2/mio.hpp")
find_package(cpr CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(examples)

# ---- Developer mode, anything after endif() will only be included if developer mode is enabled ----
if(NOT ct_search_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ct_search"
  )
endif()

include(cmake/dev-mode.cmake)
