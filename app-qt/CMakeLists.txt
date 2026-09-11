# ==============================================================================
# GUI Application
# ==============================================================================
add_executable(FujinonSX800App
    main.cpp
    MainWindow.h
    MainWindow.cpp
    widgets/ConnectionWidget.h
    widgets/ConnectionWidget.cpp
    widgets/TrafficInspectorWidget.h
    widgets/TrafficInspectorWidget.cpp
    tabs/OpticsTab.h
    tabs/OpticsTab.cpp
    tabs/ImageQualityTab.h
    tabs/ImageQualityTab.cpp
    tabs/DayNightTab.h
    tabs/DayNightTab.cpp
    tabs/OsdVideoTab.h
    tabs/OsdVideoTab.cpp
    tabs/SdCardMenuTab.h
    tabs/SdCardMenuTab.cpp
    tabs/SystemTab.h
    tabs/SystemTab.cpp
    resources.qrc
)

target_include_directories(FujinonSX800App PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(FujinonSX800App PRIVATE
    FujinonSX800Qt
    Qt6::Widgets
)

apply_compiler_flags(FujinonSX800App)

