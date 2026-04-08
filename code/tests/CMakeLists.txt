project(kemai_tests)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    client/tst_kimaiClient.cpp
    context/tst_profileManager.cpp
    misc/tst_fuzzyMatch.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    KemaiCore
    Catch2::Catch2
)

include(CTest)
include(Catch)

get_target_property(_qt6_core_loc Qt6::Core LOCATION)
get_filename_component(_qt6_bin_dir "${_qt6_core_loc}" DIRECTORY)

catch_discover_tests(${PROJECT_NAME} DL_PATHS "${_qt6_bin_dir}")
