project(KemaiUpdater)

find_package(Qt5 COMPONENTS Network CONFIG REQUIRED)

add_library(${PROJECT_NAME})

set(SRCS
        kemaiupdater.cpp)

set(PUB_HDRS
        include/kemai/kemaiupdater.h)

set(PRIV_HDRS
        kemaiupdater_p.h)

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${PRIV_HDRS} ${PUB_HDRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} Qt5::Network)
