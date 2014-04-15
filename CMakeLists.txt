project(simpleCm)

cmake_minimum_required(VERSION 2.8)

find_package(TelepathyQt4 0.9.2.1 REQUIRED)

find_package(TelepathyQt4 0.9.2.1 REQUIRED)
find_package(TelepathyQt4Service 0.9.2.1 REQUIRED)

find_package(Qt4 REQUIRED)

include_directories(
    ${QT_INCLUDES}
    ${TELEPATHY_QT4_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)

add_definitions(-DSIMPLECM_LIBRARY)

set(simpleCm_SRCS
    protocol.cpp
    protocol.h
    connection.h
    connection.cpp
    textchannel.cpp
    textchannel.h
)

add_library(simpleCm STATIC ${simpleCm_SRCS})

set_target_properties(simpleCm PROPERTIES AUTOMOC TRUE)

target_link_libraries(simpleCm
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_QT4_SERVICE_LIBRARIES}
)
