
# This function creates a library target with the specified name 
# and files. The compiler settings will be set to the following
# values
function (create_lib_target TARGET_NAME)
   
   add_library(${TARGET_NAME} STATIC)

   set_target_properties(${TARGET_NAME}
   	PROPERTIES
   		OUTPUT_NAME ${TARGET_NAME}
   		COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   		COMPILE_PDB_NAME ${TARGET_NAME}
         MSVC_RUNTIME_LIBRARY ${CTB_MSVC_RUNTIME_LIBRARY}
   )

   target_compile_options(${TARGET_NAME} PRIVATE ${CTB_COMPILE_OPTIONS})
   target_compile_options(${TARGET_NAME} PRIVATE ${CTB_WARNING_FLAGS})
   target_compile_definitions(${TARGET_NAME} PRIVATE ${CTB_COMPILE_DEFINITIONS})
endfunction()


# This function creates an EXE target with the specified name 
# The compiler settings will be set to the contents of the below
# variables, which should be defined by your preset 
function (create_exe_target TARGET_NAME)
   add_executable(${TARGET_NAME})

   set_target_properties(${TARGET_NAME}
   	PROPERTIES
   		OUTPUT_NAME ${TARGET_NAME}
   		COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   		COMPILE_PDB_NAME ${TARGET_NAME}
         MSVC_RUNTIME_LIBRARY ${CTB_MSVC_RUNTIME_LIBRARY}
   )

   target_compile_options(${TARGET_NAME} PRIVATE ${CTB_COMPILE_OPTIONS})
   target_compile_options(${TARGET_NAME} PRIVATE ${CTB_WARNING_FLAGS})
   target_compile_definitions(${TARGET_NAME} PRIVATE ${CTB_COMPILE_DEFINITIONS})
   target_link_options(${TARGET_NAME} PRIVATE ${CTB_LINK_OPTIONS})
   
endfunction()


# This function creates an Windows GUI target with the specified name 
# The compiler settings will be set to the contents of the below
# variables, which should be defined by your preset 
function (create_windows_target TARGET_NAME)
   add_executable(${TARGET_NAME} WIN32)

   set_target_properties(${TARGET_NAME}
   	PROPERTIES
   		OUTPUT_NAME ${TARGET_NAME}
   		COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   		COMPILE_PDB_NAME ${TARGET_NAME}
         MSVC_RUNTIME_LIBRARY ${CTB_MSVC_RUNTIME_LIBRARY}
         VS_DPI_AWARE "PerMonitor"
   )

   target_compile_options(${TARGET_NAME} PRIVATE ${CTB_COMPILE_OPTIONS})
   target_compile_options(${TARGET_NAME} PRIVATE ${CTB_WARNING_FLAGS})
   target_compile_definitions(${TARGET_NAME} PRIVATE ${CTB_COMPILE_DEFINITIONS})
   target_link_options(${TARGET_NAME} PRIVATE ${CTB_LINK_OPTIONS})
   
endfunction()


# add_manifest(<target> <manifest file>)
#
# Adds a manifest file (https://learn.microsoft.com/en-us/windows/win32/sbscs/application-manifests)
# to an EXE
function(add_manifest TARGET_NAME MANIFEST_FILE )

  if(NOT TARGET_NAME)
      message(FATAL_ERROR "You must provide a target")
    endif()
    if(NOT MANIFEST_FILE)
      message(FATAL_ERROR "You must provide a manifest file")
    endif()
    add_custom_command(
       TARGET ${TARGET_NAME}
       POST_BUILD
       COMMAND "mt.exe" -manifest \"${MANIFEST_FILE}\" \"-updateresource:$<TARGET_FILE:${TARGET_NAME}>\"
    )

endfunction()

