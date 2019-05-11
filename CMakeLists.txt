project(simplecm)

cmake_minimum_required(VERSION 2.8)

set(SIMPLECM_VERSION_MAJOR 0)
set(SIMPLECM_VERSION_MINOR 2)
set(SIMPLECM_VERSION_PATCH 0)

set(SIMPLECM_VERSION "${SIMPLECM_VERSION_MAJOR}.${SIMPLECM_VERSION_MINOR}.${SIMPLECM_VERSION_PATCH}")

# Add an option for building tests
option(BUILD_EXAMPLE "Enable compilation of an example app" TRUE)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Network Xml)
find_package(TelepathyQt5 0.9.6 REQUIRED)
find_package(TelepathyQt5Service 0.9.6 REQUIRED)

include(GNUInstallDirs)

add_definitions(-DSIMPLECM_LIBRARY)

set(simplecm_HEADERS
    connection.h
    protocol.h
    textchannel.h
    simplecm_export.h
)

set(simplecm_SOURCES
    connection.cpp
    protocol.cpp
    textchannel.cpp
)

add_library(simplecm-qt${QT_VERSION_MAJOR} STATIC ${simplecm_SOURCES} ${simplecm_HEADERS})

target_include_directories(simplecm-qt${QT_VERSION_MAJOR} PRIVATE
    ${TELEPATHY_QT5_INCLUDE_DIR}
)

target_link_libraries(simplecm-qt${QT_VERSION_MAJOR}
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    Qt5::Xml
    ${TELEPATHY_QT5_LIBRARIES}
    ${TELEPATHY_QT5_SERVICE_LIBRARIES}
)

set_target_properties(simplecm-qt${QT_VERSION_MAJOR} PROPERTIES AUTOMOC TRUE)

install(
    TARGETS simplecm-qt${QT_VERSION_MAJOR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Library
)

install(
    FILES ${simplecm_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplecm
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
