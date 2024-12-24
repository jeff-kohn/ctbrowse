# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(ctwinTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

create_exe_target(ctwin_test)
target_sources(ctwin_test PRIVATE source/ctwin_test.cpp)
target_link_libraries(
    ctwin_test PRIVATE
    ctwin_lib
    Catch2::Catch2WithMain
)
target_compile_features(ctwin_test PRIVATE cxx_std_17)

catch_discover_tests(ctwin_test)

# ---- End-of-file commands ----

add_folders(Test)
