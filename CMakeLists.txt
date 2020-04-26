cmake_minimum_required(VERSION 3.5)

project(SimpleCM
    VERSION 0.3.0
)

# Add an option for building tests
option(BUILD_TOOLS "Build tools" TRUE)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC TRUE)
set(QT_VERSION_MAJOR 5)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Network Xml)
find_package(TelepathyQt5 0.9.8 COMPONENTS Core Service REQUIRED)

add_subdirectory(src)

if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()
