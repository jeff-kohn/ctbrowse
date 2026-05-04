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
target_compile_options(HtmlParser PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-w>
)
# Disable clang-tidy for external library
set_target_properties(HtmlParser PROPERTIES
    CXX_CLANG_TIDY ""
    CMAKE_CXX_CPPCHECK ""
)

add_folders(deps)