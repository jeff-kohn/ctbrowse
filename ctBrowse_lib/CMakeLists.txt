set(TARGET_NAME "ctBrowse_lib")

# this call creates the target and sets compiler/linker settings and flags
create_lib_target(${TARGET_NAME})

#add an alias so it can be used with find_package or add_subdirectory
add_library(${PROJECT_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_sources(${TARGET_NAME}
   PUBLIC
      "../include/ctb/ctb.h"
      "../include/ctb/common_types.h"
      "../include/ctb/concepts.h"
      "../include/ctb/constants.h"
      "../include/ctb/CredentialManager.h"
      "../include/ctb/CredentialWrapper.h"
      "../include/ctb/ctb_format.h"
      "../include/ctb/ctb_functors.h"
      "../include/ctb/ctb_indirect.h"
      "../include/ctb/HttpDownloader.h"
      
      "../include/ctb/Error.h"
      "../include/ctb/log.h"
      "../include/ctb/utility.h"
      "../include/ctb/utility_chrono.h"
      "../include/ctb/utility_http.h"
      "../include/ctb/utility_templates.h"
      
      "../include/ctb/interfaces/DatasetEvent.h"
      "../include/ctb/interfaces/IDataset.h"
      "../include/ctb/interfaces/IDatasetEventSink.h"
      "../include/ctb/interfaces/IDatasetEventSource.h"
            
      "../include/ctb/model/CtDataset.h"
      "../include/ctb/model/CtDatasetOptions.h"
      "../include/ctb/model/CtDatasetManager.h"
      "../include/ctb/model/DatasetEventSource.h"
      "../include/ctb/model/DatasetEventHandler.h"
      "../include/ctb/model/ProReviewsCache.h"
      "../include/ctb/model/ScopedDatasetFreeze.h"
      
      "../include/ctb/tables/BottleInventoryTraits.h"
      "../include/ctb/tables/ConsumedWineTraits.h"
      "../include/ctb/tables/CtSchema.h"
      "../include/ctb/tables/PendingWineTraits.h"
      "../include/ctb/tables/PrivateNotesTraits.h"
      "../include/ctb/tables/ProReviewsCacheTraits.h"
      "../include/ctb/tables/PurchasedWineTraits.h"
      "../include/ctb/tables/ReadyToDrinkTraits.h"
      "../include/ctb/tables/table_data.h"
      "../include/ctb/tables/TastingNotesTraits.h"
      "../include/ctb/tables/TaggedWinesTraits.h"
      "../include/ctb/tables/WineListTraits.h"

      "../include/ctb/tables/detail/field_helpers.h"
      "../include/ctb/tables/detail/FieldSchema.h"
      "../include/ctb/tables/detail/FilterManager.h"
      "../include/ctb/tables/detail/ListColumn.h"
      "../include/ctb/tables/detail/MultiValueFilter.h"
      "../include/ctb/tables/detail/PropertyFilter.h"
      "../include/ctb/tables/detail/PropertyFilterPredicate.h"
      "../include/ctb/tables/detail/PropertyValue.h"
      "../include/ctb/tables/detail/SubstringFilter.h"
      "../include/ctb/tables/detail/TableRecord.h"
      "../include/ctb/tables/detail/TableSorter.h"

      "../include/ctb/tasks/PollingTask.h"
      "../include/ctb/tasks/tasks.h"
      "../include/ctb/tasks/async_tasks.h"


   PRIVATE
      "BackgroundThreadContext.h"
      "CredentialManager.cpp"
      "CredentialWrapper.cpp"
      "CtDatasetManager.cpp"
      "CtDatasetOptions.cpp"
      "HeadlessWebClient.cpp"
      "HeadlessWebClient.h"
      "HttpDownloader.cpp"
      "log.cpp"
      "ProReviewsCache.cpp"
      "ScopedHandle.h"
      "tasks.cpp"
      "utility.cpp"
      "utility_http.cpp"
      "utility_win32.cpp"
      "utility_win32.h"
      "webclient_schema.h"
 )

target_include_directories(${TARGET_NAME}
   ${warning_guard}
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_include_directories(${TARGET_NAME}
   SYSTEM
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3rdParty>"
)

# Note that since this target is a 
target_link_libraries(${TARGET_NAME}
   PUBLIC
      asio::asio
      cpr::cpr
      fmt::fmt
      glaze::glaze
      HtmlParser::HtmlParser
      spdlog::spdlog
      STDEXEC::stdexec
      "$<$<PLATFORM_ID:Windows>:Credui>"
      "$<$<PLATFORM_ID:Windows>:shlwapi>"
      "$<$<PLATFORM_ID:Windows>:Shell32.lib>"
)

if (CTB_USE_PRECOMPILED_HEADERS)
   target_precompile_headers(${TARGET_NAME}
      PRIVATE
         <algorithm>
         <array>
         <boost/algorithm/string/predicate.hpp>
         <boost/unordered/unordered_flat_map.hpp>
         <chrono>
         <concepts>
         <cpr/api.h>
         <cpr/cprtypes.h>
         <cpr/cprtypes.h>
         <cpr/response.h>
         <enchantum/enchantum.hpp>
         <enchantum/fmt_format.hpp>
         <exception>
         <expected>
         <external/csv.hpp>
         <filesystem>
         <fmt/chrono.h>
         <fmt/format.h>
         <frozen/map.h>
         <functional>
         <map>
         <memory>
         <optional>
         <ranges>
         <set>
         <span>
         <string_view>
         <string>
         <unordered_set>
         <variant>
         <vector>
   )
   message(STATUS "precompiled header support enabled for ${TARGET_NAME}")
endif()


# ---- End-of-file commands ----

# add_folders(lib)
