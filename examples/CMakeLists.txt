
create_exe_target(download_csv)
target_sources(download_csv
   PRIVATE
      "download_csv.cpp"
)
target_link_libraries(download_csv PRIVATE ct_search::cts_lib)


create_exe_target(parse_csv)
target_sources(parse_csv
   PRIVATE
      "parse_csv.cpp"
)
target_link_libraries(parse_csv PRIVATE ct_search::cts_lib)

