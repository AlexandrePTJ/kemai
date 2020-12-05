project(KemaiClient)

add_library(${PROJECT_NAME})


target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${PRIV_HDRS} ${PUB_HDRS})
target_link_libraries(${PROJECT_NAME} KemaiCore Qt::Network)
