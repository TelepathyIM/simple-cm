
set(CMAKE_AUTOUIC TRUE)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Gui Widgets Network Xml)

set(cmTest_SRCS
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui
    ManagerPreset.hpp
    PresetsLoader.cpp
    PresetsLoader.hpp
    CComboBoxDelegate.cpp
    CComboBoxDelegate.hpp
    CContactsModel.cpp
    CContactsModel.hpp
)

add_executable(simple-connection-manager-qt${QT_VERSION_MAJOR} ${cmTest_SRCS})

target_link_libraries(simple-connection-manager-qt${QT_VERSION_MAJOR} PRIVATE
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Xml
    SimpleCM::SimpleCM
)
