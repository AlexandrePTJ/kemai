project(KemaiClient)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Network CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} BASE_NAME kemai)
file(COPY
        ${CMAKE_CURRENT_BINARY_DIR}/kemai_export.h
        DESTINATION ${KEMAI_INCDIR}/kemai/)

include_directories(${KEMAI_INCDIR}/kemai)

set(SRCS
        kimaiapi.cpp
        kimaiclient.cpp
        kimaireply.cpp
        kimairequest.cpp
        kimairequestfactory.cpp
        parser.cpp)

set(PUB_HDRS
        ${KEMAI_INCDIR}/kemai/kimaiapi.h
        ${KEMAI_INCDIR}/kemai/kimaiclient.h
        ${KEMAI_INCDIR}/kemai/kimaireply.h
        ${KEMAI_INCDIR}/kemai/kimairequest.h
        ${KEMAI_INCDIR}/kemai/kimairequestfactory.h)

set(PRIV_HDRS
        kimaiclient_p.h
        parser.h)

target_sources(${PROJECT_NAME} PUBLIC ${PUB_HDRS} PRIVATE ${SRCS} ${PRIV_HDRS})
target_link_libraries(${PROJECT_NAME} Qt5::Network)
