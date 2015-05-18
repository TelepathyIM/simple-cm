
if (USE_QT4)
    find_package(Qt4 4.6.0 REQUIRED QtCore QtGui QtDBus)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core DBus Gui Widgets Network Xml)
endif()

set(cmTest_SRCS
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    CComboBoxDelegate.cpp
    CComboBoxDelegate.hpp
    CContactsModel.cpp
    CContactsModel.hpp
)

set(cmTest_UI
    MainWindow.ui
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if (USE_QT4)
    QT4_WRAP_UI(cmTest_UI_SOURCES ${cmTest_UI})
else()
    QT5_WRAP_UI(cmTest_UI_SOURCES ${cmTest_UI})
endif()

add_executable(simple-connection-manager-qt${QT_VERSION_MAJOR} ${cmTest_SRCS} ${cmTest_UI_SOURCES})

if (USE_QT4)
    target_include_directories(simple-connection-manager-qt${QT_VERSION_MAJOR} PRIVATE
        ${QT_INCLUDES}
        ${TELEPATHY_QT4_INCLUDE_DIR}
    )
    target_link_libraries(simple-connection-manager-qt${QT_VERSION_MAJOR} LINK_PRIVATE
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDBUS_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${TELEPATHY_QT4_LIBRARIES}
        ${TELEPATHY_QT4_SERVICE_LIBRARIES}
    )
else()
    target_include_directories(simple-connection-manager-qt${QT_VERSION_MAJOR} PRIVATE
        ${TELEPATHY_QT5_INCLUDE_DIR}
    )

    target_link_libraries(simple-connection-manager-qt${QT_VERSION_MAJOR} LINK_PRIVATE
        Qt5::Core
        Qt5::DBus
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::Xml
        ${TELEPATHY_QT5_LIBRARIES}
        ${TELEPATHY_QT5_SERVICE_LIBRARIES}
    )
endif()

target_link_libraries(simple-connection-manager-qt${QT_VERSION_MAJOR} LINK_PUBLIC
    simplecm-qt${QT_VERSION_MAJOR}
)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

set_target_properties(simple-connection-manager-qt${QT_VERSION_MAJOR} PROPERTIES AUTOMOC TRUE)
