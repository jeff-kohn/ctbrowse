
create_exe_target(download_csv)

target_sources(download_csv
   PRIVATE
      "download_csv.cpp"
)
target_link_libraries(download_csv PRIVATE ctwin::ctwin_lib)
