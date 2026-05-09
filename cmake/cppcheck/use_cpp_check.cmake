if (CTB_USE_CPP_CHECK)

   message (CHECK_START "Looking for CppCheck")

   find_program(CPPCHECK_PATH NAMES cppcheck HINTS $ENV{PROGRAMFILES}/cppcheck)
   if (CPPCHECK_PATH)

      set (
         CPPCHECK_SUPPRESSION_LIST 
            "${CMAKE_SOURCE_DIR}/cmake/cppcheck/cppcheck_suppressions.txt" 
            CACHE 
            FILEPATH "Text file listing check suppressions"
      )

      set (CMAKE_CXX_CPPCHECK ${CPPCHECK_PATH})
      list (APPEND CMAKE_CXX_CPPCHECK 
            "--check-level=normal"
            "--enable=warning,performance,portability,information"
            "--inline-suppr"
            "--suppressions-list=${CPPCHECK_SUPPRESSION_LIST}"
            "-I${CMAKE_SOURCE_DIR}/include\""
            "--template={file}({line}): warning: [{id}] ({severity}): {message} "
            "--quiet"
      )
      message(STATUS "CppCheck build integration is enabled for this preset.")
 
      # Need to create the directory for the cppcheck project database, because cppcheck won't create it automatically if it doesn't exist.
      set (
         CPPCHECK_PROJECT_DIR 
            "${CMAKE_BINARY_DIR}/cppcheck" 
            CACHE 
            PATH "Directory CppCheck will use for it's project database"
      )
      file ( MAKE_DIRECTORY "${CPPCHECK_PROJECT_DIR}" )

      if (MSVC)
         # VS Generator doesn't support compile-commands.json so use  the slnx instead.
         set (CPPCHECK_PROJECT_FILE "${CMAKE_BINARY_DIR}/${CMAKE_CURRENT_PROJECT}.slnx")
      else()
         set (CPPCHECK_PROJECT_FILE "${CMAKE_BINARY_DIR}/compile_commands.json")
      endif()         

      # This custom target is used to run CppCheck on the entire project using compile_commands.json. This is used during CI builds to generate 
      # alerts for GitHub Advanced Security, but can also be run locally. Results are output to an XML file, which in the case of CI builds gets 
      # converted to SARIF format and uploaded to Github. 
      add_custom_command ( 
         TARGET code_analysis POST_BUILD 
         COMMAND "${CPPCHECK_PATH}" 
            "--check-level=exhaustive"
            "--enable=all" 
            "--inconclusive" 
            "--inline-suppr" 
            "-I\"${CMAKE_SOURCE_DIR}/include\"" 
            "--template=vs"
            "--cppcheck-build-dir=\"${CPPCHECK_PROJECT_DIR}\"" 
            "--relative-paths=${CMAKE_SOURCE_DIR}"
            "--xml" 
            "--output-file=${CMAKE_BINARY_DIR}/cppcheck_results.xml" 
            "--project=${CPPCHECK_PROJECT_FILE}" 
            "--suppressions-list=${CPPCHECK_SUPPRESSION_LIST}"
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
      message(STATUS "CppCheck project scanning enabled for target 'code_analysis'")

   else()
      message(CHECK_FAIL "CppCheck was not found, 'code_analysis' target will not use CppCheck")
   endif()

endif()

