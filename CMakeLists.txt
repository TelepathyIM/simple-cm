cmake_minimum_required(VERSION 3.5)

project(SimpleCM
    VERSION 0.3.0
)

# Add an option for building tests
option(BUILD_EXAMPLE "Enable compilation of an example app" TRUE)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC TRUE)
set(QT_VERSION_MAJOR 5)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Network Xml)
find_package(TelepathyQt5 0.9.8 COMPONENTS Core Service REQUIRED)

set(simplecm_SOURCES
    connection.cpp
    connection.h
    protocol.cpp
    protocol.h
    simplecm_export.h
    textchannel.cpp
    textchannel.h
)

add_library(simplecm-qt${QT_VERSION_MAJOR} STATIC ${simplecm_SOURCES})
add_library(SimpleCM::SimpleCM ALIAS simplecm-qt${QT_VERSION_MAJOR})

target_compile_definitions(simplecm-qt${QT_VERSION_MAJOR} PRIVATE
    SIMPLECM_LIBRARY
)

set(SIMPLECM_INCLUDE_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR}/SimpleCM)

target_include_directories(simplecm-qt${QT_VERSION_MAJOR} PUBLIC
    $<INSTALL_INTERFACE:${SIMPLECM_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(simplecm-qt${QT_VERSION_MAJOR}
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    Qt5::Xml
    TelepathyQt5::Core
    TelepathyQt5::Service
)

install(
    TARGETS simplecm-qt${QT_VERSION_MAJOR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Library
)

install(
    FILES ${simplecm_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SimpleCM
    COMPONENT Devel
)

install(
    DIRECTORY SimpleCM
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
