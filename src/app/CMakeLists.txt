project(KemaiApp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Network CONFIG REQUIRED)

configure_file(kemai_version.h.in ${PROJECT_BINARY_DIR}/kemai_version.h)

add_executable(${PROJECT_NAME} ${OS_BUNDLE})
add_definitions(-DKemaiClient_EXPORTS)

set(SRCS
        activitywidget.cpp
        customerdialog.cpp
        main.cpp
        mainwindow.cpp
        settings.cpp
        settingswidget.cpp)

set(HDRS
        activitywidget.h
        customerdialog.h
        mainwindow.h
        settings.h
        settingswidget.h)

set(UIS
        activitywidget.ui
        customerdialog.ui
        mainwindow.ui
        settingswidget.ui)

set(RESX
        icons.qrc)

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${UIS} ${RESX})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets KemaiClient ${CONAN_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION .)
