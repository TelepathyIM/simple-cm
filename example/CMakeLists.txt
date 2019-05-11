
find_package(Qt5 REQUIRED COMPONENTS Core DBus Gui Widgets Network Xml)

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

qt5_wrap_ui(cmTest_UI_SOURCES ${cmTest_UI})

add_executable(simple-connection-manager-qt${QT_VERSION_MAJOR} ${cmTest_SRCS} ${cmTest_UI_SOURCES})

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

target_link_libraries(simple-connection-manager-qt${QT_VERSION_MAJOR} LINK_PUBLIC
    simplecm-qt${QT_VERSION_MAJOR}
)

include_directories(${CMAKE_BINARY_DIR})

set_target_properties(simple-connection-manager-qt${QT_VERSION_MAJOR} PROPERTIES AUTOMOC TRUE)
