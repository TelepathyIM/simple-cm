
set(simplecm_SOURCES
    connection.cpp
    connection.h
    protocol.cpp
    protocol.h
    simplecm_export.h
    service.cpp
    service.h
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
