cmake_minimum_required(VERSION 4.2)

project(
   ctBrowse
   VERSION 0.5.0.0
   DESCRIPTION "ctBrowse"
   LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
   message(FATAL_ERROR "In-source builds are not allowed")
endif()

# Options that can be set by users of this project (eg through -D on the cmake command line).
# These are pre-configured by the presets or resonable defaults, but can be overridden when needed.
option(CTB_BUILD_TESTS             "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }." ${PROJECT_IS_TOP_LEVEL})
option(CTB_CPPCHECK_BUILD_SCAN     "Enable CppCheck in-build scanning. Default: OFF. Values: { ON, OFF }." "OFF")
option(CTB_CPPCHECK_PROJECT_SCAN   "Enable CppCheck project scanning in the 'code_analysis target.   Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }." ${PROJECT_IS_TOP_LEVEL})
option(CTB_CLANG_TIDY_BUILD_SCAN   "Enable clang-tidy in-build scanning. Default: OFF. Values: { ON, OFF }." "OFF")
option(CTB_CLANG_TIDY_PROJECT_SCAN "Enable clang-tidy project scanning in the 'code_analysis target. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }." ${PROJECT_IS_TOP_LEVEL})
option(CTB_USE_PRECOMPILED_HEADERS "Use precompiled headers during build. Default: OFF. Values: { ON, OFF }." "OFF")

# So that cmake can find any additional modules we use
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
include(add_folders)
include(create_targets)
include(product-version/generate-product-version)

if(PROJECT_IS_TOP_LEVEL)
   # Only do these if this is the main project, and not if it is included through add_subdirectory
   set(CMAKE_CXX_STANDARD 23)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   # Set platform-specific toolset options
   if(WIN32)
      # By defualt cmake uses linux conventions (eg libname instead of name.lib) so override that for Windows builds.
      SET(CMAKE_FIND_LIBRARY_PREFIXES "")
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")

      set(CTB_COMPILE_OPTIONS
         /bigobj /permissive- /sdl /utf-8 /EHsc /Zc:__cplusplus /cgthreads8
         $<$<CONFIG:Debug>:/Od $<$<CXX_COMPILER_ID:MSVC>:/ZI> >
         $<$<NOT:$<CONFIG:Debug>>:/Zi /Gw /Gy /guard:cf>
         $<$<CONFIG:RelWithDebInfo>:/Ob3>
      )

      if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
         list(APPEND CTB_COMPILE_OPTIONS /std:c++latest /Zf $<$<CONFIG:Release,ReleaseMinSize,RelWithDebInfo>:/GL>)
      else()
         list(APPEND CTB_COMPILE_OPTIONS -Wno-unqualified-std-cast-call)
         set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
      endif()

      add_custom_target(code_analysis DEPENDS all)
      include(cppcheck/use_cpp_check)
      include(use_clang_tidy)

      set(CTB_COMPILE_DEFINITIONS
         _MBCS WIN32 _WIN32_WINNT=0x0A00 WINVER=0x0A00 NTDDI_VERSION=0x0A000000 _WINSOCK_DEPRECATED_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN
         wxUSE_CONFIG_NATIVE=0 wxUSE_STD_CONTAINERS=1 wxUSE_UTF8_LOCALE_ONLY=1 wxUSE_STD_STRING=1 wxUSE_STL=1 wxUSE_STL_BASED_WXSTRING=1 wxNO_IMPLICIT_WXSTRING_CONV_TO_PTR=1 wxUSE_UNICODE_UTF8=1 wxUSE_UNICODE_WCHAR=0 wxUSE_UNSAFE_WXSTRING_CONV=0
         SPDLOG_FMT_EXTERNAL=1
         $<$<CONFIG:DEBUG>:_DEBUG _ALLOW_RTCc_IN_STL>
      )

      set(CTB_LINK_OPTIONS
         /DEBUG:FULL /guard:cf $<$<CXX_COMPILER_ID:MSVC>:/CGTHREADS:8>
         $<$<CONFIG:Release,RelWithDebInfo,ReleaseMinSize>:/LTCG>
      )

      set(CTB_WARNING_FLAGS
         /W4 /WX
         $<$<CXX_COMPILER_ID:MSVC>:/w14165 /w14186 /w14263 /w14265 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14640 /w14826 /w14905 /w14906 /w14928 /w24826 /w34265 /w34287 /w34619 /w34640 /w44242 /w44254 /w44263 /w44296 /w44365 /w44388 /w45038 /wd4100 /wd4714 /external:anglebrackets /external:W0>
      )
   endif()

else()
   # If this project is being included through add_subdirectory, provide a default-enabled option to use the SYSTEM modifier for ctBrowse's includes,
   # which will disable warnings from ctBrowse's headers in the parent project. And we won't set any compiler/linker flags either.
   option(ctBrowse_INCLUDES_WITH_SYSTEM "Use SYSTEM modifier for ctBrowse's includes, disabling warnings" ON)
   mark_as_advanced(ctBrowse_INCLUDES_WITH_SYSTEM)

   if(ctBrowse_INCLUDES_WITH_SYSTEM)
      set(warning_guard SYSTEM)
   endif()
endif()

add_compile_definitions(STDEXEC_ASIO_USES_STANDALONE=1 STDEXEC_ASIO_USES_BOOST=0)
find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glaze CONFIG)
find_package(spdlog CONFIG REQUIRED)
find_package(stdexec CONFIG REQUIRED)

add_subdirectory(ctBrowse_app)
add_subdirectory(ctBrowse_lib)

if(CTB_BUILD_TESTS)
   enable_testing()
   add_subdirectory("tests")
endif()

# include(CMakePrintHelpers)
