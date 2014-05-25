project(simpleCm)

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

set(simpleCm_HEADERS
    connection.h
    protocol.h
    textchannel.h
)

set(simpleCm_SOURCES
    connection.cpp
    protocol.cpp
    textchannel.cpp
)

add_library(simpleCm STATIC ${simpleCm_SOURCES} ${simpleCm_HEADERS})

set_target_properties(simpleCm PROPERTIES AUTOMOC TRUE)

target_link_libraries(simpleCm
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_QT4_SERVICE_LIBRARIES}
)

install(
    TARGETS simpleCm
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Library
)

install(
    FILES ${simpleCm_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simpleCm
    COMPONENT Devel
)

install(
    DIRECTORY SimpleCM
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
