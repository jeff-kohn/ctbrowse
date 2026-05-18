if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
   # clang-tidy doesn't work with other compilers if you're using precompiled headers or can't generate compile_commands.json, which is lame

   if (CTB_CLANG_TIDY_BUILD_SCAN)
      message(CHECK_START "Looking for clang-tidy")
      find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy clang-tidy.exe)
   
      if (CMAKE_CXX_CLANG_TIDY)
         message(CHECK_PASS "found at '${CMAKE_CXX_CLANG_TIDY}'")
         list(
            APPEND CMAKE_CXX_CLANG_TIDY
            "-p"
            ${CMAKE_BINARY_DIR}
            "--use-color"
            "--quiet"
         )
         message(STATUS "clang-tidy build integration is enabled for this preset.")
      else()
         message(CHECK_FAIL "clang-tidy build integration could not be enabled, clang-tidy was not found")
      endif()
   endif()

   if (CTB_CLANG_TIDY_PROJECT_SCAN)
         message (CHECK_START "Looking for run-clang-tidy")
         find_program(RUN_CLANG_TIDY NAMES run-clang-tidy run-clang-tidy.py)

         if (RUN_CLANG_TIDY)

            message(CHECK_PASS "found at '${RUN_CLANG_TIDY}'")

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
   endif()

endif()

