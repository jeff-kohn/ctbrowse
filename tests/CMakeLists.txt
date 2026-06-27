# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(ctBrowse_tests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

create_exe_target(ctb_test)

target_sources(ctb_test
   PRIVATE
      "ctb_test.cpp"
)

target_link_libraries(ctb_test
   PRIVATE
      ctBrowse_lib
      Catch2::Catch2WithMain
)

catch_discover_tests(ctb_test)
add_folders(Test)
