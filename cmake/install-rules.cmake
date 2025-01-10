install(
    TARGETS ct_search_exe
    RUNTIME COMPONENT ct_search_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
