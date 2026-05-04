vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO jeff-kohn/HtmlParser
    REF master
    SHA512 a3ea60d3d00be8aa78bb0e82c0295d4b29ee0dbc2684691730f559e51780750aa37f701b7f669d54ac23ac571198dbc5a76dc8fc6d5d8ba4d9baee2bc660ba06
    HEAD_REF master
)
vcpkg_find_acquire_program(PKGCONFIG)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DHTMLPARSER_BUILD_TESTS=OFF
        -DHTMLPARSER_BUILD_BENCHMARKS=OFF
        -DHTMLPARSER_BUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")