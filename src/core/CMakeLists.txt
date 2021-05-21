project(KemaiCore)

add_library(${PROJECT_NAME})

configure_file(kemai_version.h.in ${PROJECT_BINARY_DIR}/kemai_version.h)

set(SRCS
        client/kimaiapi.cpp
        client/kimaiclient.cpp
        client/kimaireply.cpp
        client/kimairequest.cpp
        client/kimairequestfactory.cpp
        client/parser.cpp
        updater/kemaiupdater.cpp
        kemaisession.cpp
        settings.cpp)

set(PUB_HDRS
        client/kimaiapi.h
        client/kimaiclient.h
        client/kimaireply.h
        client/kimairequest.h
        client/kimairequestfactory.h
        updater/kemaiupdater.h
        kemaisession.h
        settings.h)

set(PRIV_HDRS
        client/kimaiclient_p.h
        client/parser.h
        updater/kemaiupdater_p.h)

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${PRIV_HDRS} ${PUB_HDRS})
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} Qt::Core Qt::Network)
