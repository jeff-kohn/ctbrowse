
create_exe_target(download_csv)
target_sources(download_csv
   PRIVATE
      "download_csv.cpp"
)
target_link_libraries(download_csv PRIVATE ctBrowse::ctBrowse_lib)


create_exe_target(parse_csv)
target_sources(parse_csv
   PRIVATE
      "parse_csv.cpp"
)
target_link_libraries(parse_csv PRIVATE ctBrowse::ctBrowse_lib)


# find_package(wxWidgets CONFIG REQUIRED)
# 
# create_windows_target(wxsample)
# target_sources(wxsample
#    PRIVATE
#       "wrapsizer.cpp"
# )
# target_link_libraries(wxsample 
#    PRIVATE 
#      wxWidgets::wxWidgets
# )

add_folders(examples)

