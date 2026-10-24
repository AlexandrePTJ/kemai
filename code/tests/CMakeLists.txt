project(kemai_tests)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    mocks/fakeKimaiClient.h
    client/tst_kimaiCache.cpp
    client/tst_kimaiClient.cpp
    context/tst_profileManager.cpp
    context/tst_sessionController.cpp
    misc/tst_formatHelpers.cpp
    misc/tst_fuzzyMatch.cpp
    misc/tst_logFiles.cpp
    misc/tst_windowPlacement.cpp
    model/tst_timesheetModel.cpp
    storage/tst_appSettingsStore.cpp
    storage/tst_profileStore.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    KemaiCore
    Catch2::Catch2
    spdlog::spdlog # tests capture or write logs themselves (core links it privately)
)

include(CTest)
include(Catch)

get_target_property(_qt6_core_loc Qt6::Core LOCATION)
get_filename_component(_qt6_bin_dir "${_qt6_core_loc}" DIRECTORY)

catch_discover_tests(${PROJECT_NAME} DL_PATHS "${_qt6_bin_dir}")
