
create_exe_target(download_xml)

target_sources(download_xml
   PRIVATE
      "download_xml.cpp"
)
target_link_libraries(download_xml PRIVATE ctwin::ctwin_lib cpr::cpr)