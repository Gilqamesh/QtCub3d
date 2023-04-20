cmake_minimum_required(VERSION 3.5)

project(Cub3d VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/utils.h
    src/renderer_widget.cpp src/renderer_widget.h src/renderer_widget.ui
    src/defs.h
    src/camera.h src/camera.cpp
    src/app/app_widget.cpp src/app/app_widget.h
    src/mvcs/map/map_model.cpp src/mvcs/map/map_model.h
    src/mvcs/map_editor/map_editor_mvc.h src/mvcs/map_editor/map_editor_mvc.cpp
    src/mvcs/map_editor/map_editor_view.h src/mvcs/map_editor/map_editor_view.cpp
    src/mvcs/map_editor/map_editor_delegate.h src/mvcs/map_editor/map_editor_delegate.cpp
    src/mvcs/mvcs_defs.h
    src/v2.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Cub3d
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Cub3d APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Cub3d SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Cub3d
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Cub3d PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::OpenGLWidgets)

set_target_properties(Cub3d PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Cub3d
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Cub3d)
endif()
