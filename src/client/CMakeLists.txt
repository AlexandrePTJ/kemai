project(KemaiClient)

find_package(Qt5 COMPONENTS Network CONFIG REQUIRED)

add_library(${PROJECT_NAME})

set(SRCS
        kimaiapi.cpp
        kimaiclient.cpp
        kimaireply.cpp
        kimairequest.cpp
        kimairequestfactory.cpp
        parser.cpp)

set(PUB_HDRS
        include/kemai/kimaiapi.h
        include/kemai/kimaiclient.h
        include/kemai/kimaireply.h
        include/kemai/kimairequest.h
        include/kemai/kimairequestfactory.h)

set(PRIV_HDRS
        kimaiclient_p.h
        parser.h)

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${PRIV_HDRS} ${PUB_HDRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} Qt5::Network CONAN_PKG::spdlog)
