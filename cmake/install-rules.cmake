install(
    TARGETS ctwin_exe
    RUNTIME COMPONENT ctwin_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
