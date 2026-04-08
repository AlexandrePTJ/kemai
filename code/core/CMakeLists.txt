project(KemaiCore)

add_library(${PROJECT_NAME} STATIC)

set(KEMAI_LIB_SOURCES
        context/profileController.h
        context/profileController.cpp
        client/kimaiSystem.h
        client/kimaiSystem.cpp
        client/kimaiResources.h
        client/kimaiResources.cpp
        client/kimaiTimesheets.h
        client/kimaiTimesheets.cpp
        client/kimaiCache.h
        client/kimaiCache.cpp
        client/kimaiClient.h
        client/kimaiClient_p.h
        client/kimaiClient.cpp
        data/appSettings.cpp
        data/appSettings.h
        data/profile.cpp
        data/profile.h
        misc/formatHelpers.h
        misc/formatHelpers.cpp
        misc/fuzzyMatch.h
        misc/fuzzyMatch.cpp
        misc/jsonHelpers.h
        misc/jsonHelpers.cpp
        misc/pathHelpers.h
        misc/pathHelpers.cpp
        model/profile.cpp
        model/profile.h
        model/timesheetModel.cpp
        model/timesheetModel.h
        storage/appSettingsStore.cpp
        storage/appSettingsStore.h
        storage/profileStore.cpp
        storage/profileStore.h
        storage/secretStorage.cpp
        storage/secretStorage.h
)

configure_file(kemaiConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/kemaiConfig.h)

target_sources(${PROJECT_NAME} PRIVATE ${KEMAI_LIB_SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
        PUBLIC Qt6::Network
        PRIVATE spdlog::spdlog fmt::fmt magic_enum::magic_enum qt6keychain)
