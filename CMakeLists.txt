project(simplecm)

cmake_minimum_required(VERSION 2.8)

set(SIMPLECM_VERSION_MAJOR 0)
set(SIMPLECM_VERSION_MINOR 1)
set(SIMPLECM_VERSION_PATCH 0)

set(SIMPLECM_VERSION "${SIMPLECM_VERSION_MAJOR}.${SIMPLECM_VERSION_MINOR}.${SIMPLECM_VERSION_PATCH}")

find_package(TelepathyQt4 0.9.2.1 REQUIRED)

find_package(TelepathyQt4 0.9.2.1 REQUIRED)
find_package(TelepathyQt4Service 0.9.2.1 REQUIRED)

find_package(Qt4 REQUIRED)

include(GNUInstallDirs)

include_directories(
    ${QT_INCLUDES}
    ${TELEPATHY_QT4_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)

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

add_library(simplecm STATIC ${simplecm_SOURCES} ${simplecm_HEADERS})

set_target_properties(simplecm PROPERTIES AUTOMOC TRUE)

target_link_libraries(simplecm
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_QT4_SERVICE_LIBRARIES}
)

install(
    TARGETS simplecm
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
