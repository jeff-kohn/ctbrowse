# only do the in-built scanning for the presets that request it.
if(CTB_ENABLE_CLANG_TIDY)
   message(CHECK_START "Looking for clang-tidy")
   find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy clang-tidy.exe)

   if(CMAKE_CXX_CLANG_TIDY)
      message(CHECK_PASS "clang-tidy found at '${CMAKE_CXX_CLANG_TIDY}'")
      list(
         APPEND CMAKE_CXX_CLANG_TIDY
         "--use-color"
         "--header-filter=.*/cts/**"
         "--quiet"
      )
      message(STATUS "clang-tidy build integration is enabled for this preset.")
   else()
      message(CHECK_FAIL "clang-tidy build integration is enabled, but clang-tidy was not found")
   endif()
else()
   message(STATUS "clang-tidy build integration disabled for this build preset")
endif()