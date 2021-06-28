add_subdirectory(core)
add_subdirectory(app)

if (BUILD_QMLAPP AND Qt6_FOUND)
    add_subdirectory(qmlapp)
endif ()
