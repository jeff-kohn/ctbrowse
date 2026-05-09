if (CTB_USE_CLANG_TIDY)

   message(CHECK_START "Looking for clang-tidy")

   find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy clang-tidy.exe)
   if (CMAKE_CXX_CLANG_TIDY)

      message(CHECK_PASS "clang-tidy found at '${CMAKE_CXX_CLANG_TIDY}'")
      list(
         APPEND CMAKE_CXX_CLANG_TIDY
         "-p"
         ${CMAKE_BINARY_DIR}
         "--use-color"
         "--header-filter=.*/ctb/**"
         "--quiet"
      )
      message(STATUS "clang-tidy build integration is enabled for this preset.")

      # If clang-tidy is enabled and found, add run-clang-tidy to code_analysis target
      message (CHECK_START "Looking for run-clang-tidy")
      find_program(RUN_CLANG_TIDY NAMES run-clang-tidy run-clang-tidy.py)
      if (RUN_CLANG_TIDY)

         message(CHECK_PASS "run-clang-tidy found at '${RUN_CLANG_TIDY}'")

         # When 'code_analysis' target is built/run, clang-tidy will be run for the entire project using compile_commands.json
         add_custom_command(TARGET code_analysis POST_BUILD
            COMMAND "${CMAKE_SOURCE_DIR}/scripts/Run-ClangTidyProjectScan.ps1"
                     "--SourceDir"
                     "${CMAKE_SOURCE_DIR}"
                     "--BuildDir"
                     "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         )
         message (STATUS "run-clang-tidy project scanning enabled for target 'code_analysis'")
      else()
         message (CHECK_FAIL "run-clang-tidy was not found, 'code_analysis' target will not use clang-tidy")
      endif()

   else()
      message(CHECK_FAIL "clang-tidy build integration is enabled, but clang-tidy was not found")
   endif()
else()
   message(STATUS "clang-tidy build integration disabled for this build preset. run-clang-tidy will not be added to code_analysis target")
endif()

