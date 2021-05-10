add_subdirectory(core)
add_subdirectory(app)

if (BUILD_QMLAPP)
    add_subdirectory(qmlapp)
endif()
