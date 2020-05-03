
set(simplecm_SOURCES
    Chat.cpp
    Chat.hpp
    connection.cpp
    connection.h
    protocol.cpp
    protocol.h
    simplecm_export.h
    service.cpp
    service.h
    ServiceLowLevel.cpp
    ServiceLowLevel_p.h
    ServiceLowLevel.h
    textchannel.cpp
    textchannel.h
)

set(public_HEADERS
    Chat.hpp
    simplecm_export.h
    service.h
    ServiceLowLevel.h
)

add_library(simplecm-qt${QT_VERSION_MAJOR} STATIC ${simplecm_SOURCES})
add_library(SimpleCM::SimpleCM ALIAS simplecm-qt${QT_VERSION_MAJOR})

target_compile_definitions(simplecm-qt${QT_VERSION_MAJOR} PUBLIC
    SIMPLECM_STATIC
)
target_compile_definitions(simplecm-qt${QT_VERSION_MAJOR} PRIVATE
    BUILD_SIMPLECM_LIB
)

set(SIMPLECM_INCLUDE_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR}/SimpleCM)

target_include_directories(simplecm-qt${QT_VERSION_MAJOR} PUBLIC
    $<INSTALL_INTERFACE:${SIMPLECM_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(simplecm-qt${QT_VERSION_MAJOR} PUBLIC
    Qt5::Core
    Qt5::DBus
)

if (TARGET TelepathyQt5::Core)
    target_link_libraries(simplecm-qt${QT_VERSION_MAJOR} PUBLIC
        TelepathyQt5::Core
        TelepathyQt5::Service
    )
else()
    target_link_libraries(simplecm-qt${QT_VERSION_MAJOR} PUBLIC
        telepathy-qt5
        telepathy-qt5-service
        Qt5::Xml
        Qt5::Network
    )
endif()

install(
    TARGETS simplecm-qt${QT_VERSION_MAJOR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Library
)

install(
    FILES ${public_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SimpleCM
    COMPONENT Devel
)

install(
    DIRECTORY SimpleCM
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
