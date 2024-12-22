cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ctwin
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    ctwin_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    ctwin_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(ctwin_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
target_link_libraries(ctwin_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(ctwin_exe source/main.cpp)
add_executable(ctwin::exe ALIAS ctwin_exe)

set_property(TARGET ctwin_exe PROPERTY OUTPUT_NAME ctwin)

target_compile_features(ctwin_exe PRIVATE cxx_std_17)

target_link_libraries(ctwin_exe PRIVATE ctwin_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
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
