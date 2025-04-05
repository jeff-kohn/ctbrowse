cmake_minimum_required (VERSION 3.30)

project(
    ctBrowse
    VERSION 0.1.0.0
    DESCRIPTION "ctBrowse"
    LANGUAGES CXX
)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

include(cmake/prelude.cmake)
include(FetchContent)

# So that cmake can find any additional modules we've written (stuff like Find modules, export targets, etc)
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(add-target)
include(product-version/generate-product-version)
include(use-clang-tidy)
include(variables)

# export compile_commands.json for tools like clang-tidy and cppcheck
set (CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") 

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (PROJECT_IS_TOP_LEVEL)
   include(folders)
endif()

include(FetchContent)


# HTML parser library
set(CMAKE_MSVC_RUNTIME_LIBRARY ${CTB_MSVC_RUNTIME_LIBRARY})
option(HTMLPARSER_BUILD_TESTS "Build tests" OFF)
option(HTMLPARSER_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(HTMLPARSER_BUILD_EXAMPLES "Build examples" OFF)
FetchContent_Declare(
        HtmlParser
        GIT_REPOSITORY https://github.com/JustCabbage/HtmlParser.git
        GIT_TAG master
        SYSTEM
)
FetchContent_MakeAvailable(HtmlParser)
target_compile_options(HtmlParser PRIVATE "-w")

# libcoro
#set(LIBCORO_EXTERNAL_DEPENDENCIES ON)
#set(LIBCORO_FEATURE_TLS OFF)
#set(LIBCORO_BUILD_TESTS OFF)
#set(LIBCORO_BUILD_EXAMPLES OFF)
#FetchContent_Declare(
#    libcoro
#    GIT_REPOSITORY https://github.com/jbaldwin/libcoro.git
#    GIT_TAG        main
#    SYSTEM
#)
#FetchContent_MakeAvailable(libcoro)
#target_compile_options(libcoro PRIVATE "-w")


find_package(cpr CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(examples)

# ---- Developer mode, anything after endif() will only be included if developer mode is enabled ----5
if(NOT ctBrowse_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ctbrowse"
  )
endif()

include(dev-mode)
