cmake_minimum_required(VERSION 3.16)
project(GradMesh VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS OpenGL OpenGLWidgets Widgets)

qt_add_executable(GradMesh WIN32 MACOSX_BUNDLE
    main.cpp
    mainview.cpp mainview.h
    mainwindow.cpp mainwindow.h mainwindow.ui
    settings.h




    model/hermite.h model/hermite.cpp
    model/basicdatatypes.h
    model/mesh.h model/mesh.cpp
    renderer.h renderer.cpp
    resources.qrc

)
target_link_libraries(GradMesh PRIVATE
    Qt::Core
    Qt::Gui
)

if((QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(GradMesh PRIVATE
        Qt::OpenGL
        Qt::OpenGLWidgets
        Qt::Widgets
    )
endif()

install(TARGETS GradMesh
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
