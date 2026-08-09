include_guard(GLOBAL)

function(get_vcpkg_triplet_variables)
  include("${CMAKE_CURRENT_LIST_DIR}/${VCPKG_TARGET_TRIPLET}.cmake")
  # Be carefull here you don't want to pull in all variables from the triplet!
  # Port is not defined!
  set(VCPKG_CRT_LINKAGE "${VCPKG_CRT_LINKAGE}" PARENT_SCOPE) # This is also forwarded by vcpkg itself
endfunction()

get_vcpkg_triplet_variables()

# This project does not use C++20/23 modules. Disable CMake's automatic module dependency
# scanning (enabled by default for Clang + Ninja when CXX_STANDARD >= 20), which otherwise
# generates a .modmap dyndep file per translation unit and can inject scan-derived flags that
# are not visible in compile_commands.json, causing spurious PCH/predefined-macro mismatches.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE BOOL "")

# set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")

# Set Windows definitions:
set(windows_defs "/DWIN32")
if(VCPKG_TARGET_ARCHITECTURE STREQUAL x64)
  string(APPEND windows_defs " /D_WIN64")
endif()
string(APPEND windows_defs " /D_WIN32_WINNT=0x0A00 /DWINVER=0x0A00") # tweak for targeted windows

# Try to ignore /WX and -werror; A lot of ports mess up the compiler detection and add wrong flags!
set(ignore_werror "/WX-")
cmake_language(DEFER CALL add_compile_options "/WX-") # make sure the flag is added at the end!

# Pin the explicit clang target triple so clang-cl invocations are deterministic regardless of
# whether CMake is launched from the VS IDE or a command-line/script build. Without this,
# clang-cl's implicit default-triple resolution can differ subtly between callers, causing
# "definition of macro ... does not match definition in precompiled header" warnings.
if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
  set(CLANG_TARGET_TRIPLE "--target=amd64-pc-windows-msvc")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
  set(CLANG_TARGET_TRIPLE "--target=i686-pc-windows-msvc")
else()
  message(FATAL_ERROR "Unsupported VCPKG_TARGET_ARCHITECTURE: \"${VCPKG_TARGET_ARCHITECTURE}\".")
endif()

# Set runtime library.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<STREQUAL:${VCPKG_CRT_LINKAGE},dynamic>:DLL>" CACHE STRING "")
if(VCPKG_CRT_LINKAGE STREQUAL "dynamic")
  set(VCPKG_CRT_FLAG "/MD")
elseif(VCPKG_CRT_LINKAGE STREQUAL "static")
  set(VCPKG_CRT_FLAG "/MT")
else()
  message(FATAL_ERROR "Invalid VCPKG_CRT_LINKAGE: \"${VCPKG_CRT_LINKAGE}\".")
endif()
 
# Set charset flag.
set(CHARSET_FLAG "/utf-8")
set(CMAKE_CL_NOLOGO "/nologo" CACHE STRING "")

# Set compiler.
find_program(CLANG-CL_EXECUTBALE NAMES "clang-cl" "clang-cl.exe" PATHS "$ENV{ProgramFiles}/LLVM" "$ENV{LLVMInstallDir}" PATH_SUFFIXES "bin" NO_DEFAULT_PATH)
find_program(CLANG-CL_EXECUTBALE NAMES "clang-cl" "clang-cl.exe" PATHS "$ENV{ProgramFiles}/LLVM" "$ENV{LLVMInstallDir}" PATH_SUFFIXES "bin" )

if(NOT CLANG-CL_EXECUTBALE)
  message(SEND_ERROR "clang-cl was not found!") # Not a FATAL_ERROR due to being a toolchain!
endif()

get_filename_component(LLVM_BIN_DIR "${CLANG-CL_EXECUTBALE}" DIRECTORY)
list(INSERT CMAKE_PROGRAM_PATH 0 "${LLVM_BIN_DIR}")

set(CMAKE_C_COMPILER "${CLANG-CL_EXECUTBALE}" CACHE STRING "")
set(CMAKE_CXX_COMPILER "${CLANG-CL_EXECUTBALE}" CACHE STRING "")
set(CMAKE_AR "${LLVM_BIN_DIR}/llvm-lib.exe" CACHE STRING "")
set(CMAKE_LINKER "${LLVM_BIN_DIR}/lld-link.exe" CACHE STRING "")
set(CMAKE_ASM_MASM_COMPILER "ml64.exe" CACHE STRING "")
set(CMAKE_RC_COMPILER "rc.exe" CACHE STRING "")
set(CMAKE_MT "mt.exe" CACHE STRING "")

set(CLANG_C_LTO_FLAGS "-fuse-ld=lld-link")
set(CLANG_CXX_LTO_FLAGS "-fuse-ld=lld-link")
if(VCPKG_USE_LTO)
  set(CLANG_C_LTO_FLAGS "-flto -fuse-ld=lld-link")
  set(CLANG_CXX_LTO_FLAGS "-flto -fuse-ld=lld-link -fwhole-program-vtables")
endif()

set(CMAKE_C_FLAGS "${CMAKE_CL_NOLOGO} ${windows_defs} ${CLANG_TARGET_TRIPLE} ${VCPKG_C_FLAGS} ${CLANG_FLAGS} ${CHARSET_FLAG} ${ignore_werror}" CACHE STRING "")
set(CMAKE_C_FLAGS_DEBUG "/Od /Ob0 /GS /RTC1 /FC ${VCPKG_C_FLAGS_DEBUG} ${VCPKG_CRT_FLAG}d ${VCPKG_DBG_FLAG} /D_DEBUG" CACHE STRING "")
set(CMAKE_C_FLAGS_RELEASE "/O2 /Oi ${CLANG_FLAGS_RELEASE} ${VCPKG_C_FLAGS_RELEASE} ${VCPKG_CRT_FLAG} ${CLANG_C_LTO_FLAGS} ${VCPKG_DBG_FLAG} /DNDEBUG" CACHE STRING "")
set(CMAKE_C_FLAGS_MINSIZEREL "/O1 /Oi /Ob1 /GS- ${CLANG_FLAGS_RELEASE} ${VCPKG_C_FLAGS_RELEASE} ${VCPKG_CRT_FLAG} ${CLANG_C_LTO_FLAGS} /DNDEBUG" CACHE STRING "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "/O2 /Oi /Ob1 /GS- ${CLANG_FLAGS_RELEASE} ${VCPKG_C_FLAGS_RELEASE} ${VCPKG_CRT_FLAG} ${CLANG_C_LTO_FLAGS} ${VCPKG_DBG_FLAG} /DNDEBUG" CACHE STRING "")

set(CMAKE_CXX_FLAGS "${CMAKE_CL_NOLOGO} /EHsc /GR ${windows_defs} ${CLANG_TARGET_TRIPLE} ${VCPKG_CXX_FLAGS} ${CLANG_FLAGS} ${CHARSET_FLAG} ${std_cxx_flags} ${ignore_werror}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /FC ${VCPKG_CXX_FLAGS_DEBUG}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${VCPKG_CXX_FLAGS_RELEASE} ${CLANG_CXX_LTO_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${VCPKG_CXX_FLAGS_RELEASE} ${CLANG_CXX_LTO_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${VCPKG_CXX_FLAGS_RELEASE} ${CLANG_CXX_LTO_FLAGS}" CACHE STRING "")

# Remove duplicated whitespaces
foreach(flag_suf IN ITEMS ";_DEBUG;_RELEASE;_MINSIZEREL;_RELWITHDEBINFO")
    string(REGEX REPLACE " +" " " CMAKE_C_FLAGS${flag} "${CMAKE_C_FLAGS${flag}}")
    string(REGEX REPLACE " +" " " CMAKE_CXX_FLAGS${flag} "${CMAKE_CXX_FLAGS${flag}}")
endforeach()
unset(flag_suf)

# Set linker flags.
foreach(linker IN ITEMS "SHARED" "MODULE" "EXE")
  set(CMAKE_${linker}_LINKER_FLAGS_INIT "${CMAKE_CL_NOLOGO} /INCREMENTAL:NO /Brepro ${VCPKG_LINKER_FLAGS}" CACHE STRING "")
  set(CMAKE_${linker}_LINKER_FLAGS "${CMAKE_CL_NOLOGO} /INCREMENTAL:NO /Brepro ${VCPKG_LINKER_FLAGS}" CACHE STRING "")
  set(CMAKE_${linker}_LINKER_FLAGS_DEBUG "/DEBUG:FULL ${VCPKG_LINKER_FLAGS_DEBUG}" CACHE STRING "")
  set(CMAKE_${linker}_LINKER_FLAGS_RELEASE "/DEBUG /OPT:REF /OPT:ICF ${VCPKG_LINKER_FLAGS_RELEASE}" CACHE STRING "")
  set(CMAKE_${linker}_LINKER_FLAGS_MINSIZEREL "/DEBUG /OPT:REF /OPT:ICF" CACHE STRING "")
  set(CMAKE_${linker}_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG:FULL /OPT:REF /OPT:ICF" CACHE STRING "")
endforeach()


foreach(lang IN ITEMS C CXX)
  foreach(linker IN ITEMS "SHARED" "MODULE" "EXE")
    set(CMAKE_${lang}_${linker}_LINKER_FLAGS "${CMAKE_${linker}_LINKER_FLAGS}" CACHE STRING "")
    set(CMAKE_${lang}_${linker}_LINKER_FLAGS_DEBUG "${CMAKE_${linker}_LINKER_FLAGS_DEBUG}" CACHE STRING "")
    set(CMAKE_${lang}_${linker}_LINKER_FLAGS_RELEASE "${CMAKE_${linker}_LINKER_FLAGS_RELEASE}" CACHE STRING "")
    set(CMAKE_${lang}_${linker}_LINKER_FLAGS_MINSIZEREL "${CMAKE_${linker}_LINKER_FLAGS_MINSIZEREL}" CACHE STRING "")
    set(CMAKE_${lang}_${linker}_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_${linker}_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "")
  endforeach()
endforeach()
unset(linker)
unset(lang)

# Set assembler flags.
set(CMAKE_ASM_MASM_FLAGS_INIT "${CMAKE_CL_NOLOGO}")

# Set resource compiler flags.
set(CMAKE_RC_FLAGS_INIT "-c65001 ${windows_defs}")
set(CMAKE_RC_FLAGS_DEBUG_INIT "-D_DEBUG")

# Setup try_compile correctly. Requires all variables required by the toolchain. 
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES VCPKG_CRT_LINKAGE 
                                                 VCPKG_C_FLAGS VCPKG_CXX_FLAGS
                                                 VCPKG_C_FLAGS_DEBUG VCPKG_CXX_FLAGS_DEBUG
                                                 VCPKG_C_FLAGS_RELEASE VCPKG_CXX_FLAGS_RELEASE
                                                 VCPKG_LINKER_FLAGS VCPKG_LINKER_FLAGS_RELEASE VCPKG_LINKER_FLAGS_DEBUG
                                                 VCPKG_SET_CHARSET_FLAG
                                                 VCPKG_USE_LTO
                                                 )

# Remove variables which are not needed anymore
unset(CHARSET_FLAG)
unset(CLANG_FLAGS)
unset(CLANG_C_LTO_FLAGS)
unset(CLANG_CXX_LTO_FLAGS)
unset(windows_defs)
unset(ignore_werror)
unset(VCPKG_DBG_FLAG)
unset(VCPKG_CRT_FLAG)
unset(CLANG_TARGET_TRIPLE)
