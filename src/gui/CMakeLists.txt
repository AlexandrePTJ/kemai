project(Kemai)

add_executable(${PROJECT_NAME} ${OS_BUNDLE})

set(SRCS)

set(HDRS)





if (WIN32)
    list(APPEND SRCS ${CMAKE_SOURCE_DIR}/bundle/windows/kemai.rc)
elseif (APPLE)
    set(KEMAI_ICNS "${CMAKE_SOURCE_DIR}/bundle/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/macos/Info.plist)
endif ()

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${UIS} ${RESX} ${KEMAI_ICNS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt::Widgets KemaiCore)
