

create_windows_target(cts_win)

target_sources(cts_win
   PRIVATE
     "App.cpp"
     "App.h"
     "MainFrame.cpp"
     "MainFrame.h"
     "TableSyncDialog.cpp"
     "TableSyncDialog.h"
     "generated/MainFrameBase.cpp"
     "generated/MainFrameBase.h"
     "generated/TableSyncDlgBase.cpp"
     "generated/TableSyncDlgBase.h"
     "utf8.manifest"
)

find_package(wxWidgets CONFIG REQUIRED)

target_link_libraries(cts_win
   PRIVATE
      ct_search::cts_lib
      wxWidgets::wxWidgets
)


# ---- End-of-file commands ----
add_folders(src)
