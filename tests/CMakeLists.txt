# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(ctBrowseTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

create_exe_target(cts_test)

target_sources(cts_test
   PRIVATE
      "source/cts_test.cpp"
)

target_link_libraries(cts_test
   PRIVATE
      ctBrowse_lib
      Catch2::Catch2WithMain
)

catch_discover_tests(cts_test)


# ---- End-of-file commands ----

add_folders(Test)
