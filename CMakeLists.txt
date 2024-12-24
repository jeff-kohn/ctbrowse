cmake_minimum_required (VERSION 3.25)

include(cmake/prelude.cmake)

project(
    ctwin
    VERSION 0.1.0
    DESCRIPTION "CellarTracker Inventory Search"
    LANGUAGES CXX
)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/add_target.cmake)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (TOP_LEVEL_PROJECT)
   # Let's ensure -std=c++xx instead of -std=g++xx
   set(CMAKE_CXX_EXTENSIONS OFF)

   # Let's nicely support folders in IDEs
   set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# ---- Developer mode ----
if(NOT ctwin_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ctwin"
  )
endif()

include(cmake/dev-mode.cmake)

find_package(cpr CONFIG REQUIRED)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(examples)
