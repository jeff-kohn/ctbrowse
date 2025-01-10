

create_exe_target_win(cts_win)

target_sources(cts_win
   PRIVATE
     "App.cpp"
     "App.h"
)

find_package(wxWidgets CONFIG REQUIRED)

target_link_libraries(cts_win
   PRIVATE
      ct_search::cts_lib
      wxWidgets::wxWidgets
)


# ---- End-of-file commands ----
add_folders(src)
