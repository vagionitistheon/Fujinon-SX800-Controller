cmake_minimum_required(VERSION 3.16)
project(FujinonSX800Controller VERSION 2.12.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warning flags as per project rules
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror=return-type)
endif()

find_package(Threads REQUIRED)
find_package(glog REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

# ==============================================================================
# 1. Core Library (Pure C++17, zero Qt dependency)
# ==============================================================================
add_library(FujinonSX800Core STATIC
    libs/FujinonSX800Core/PelcoDTypes.h
    libs/FujinonSX800Core/CameraStatus.h
    libs/FujinonSX800Core/ITransport.h
    libs/FujinonSX800Core/PelcoDFrame.h
    libs/FujinonSX800Core/PelcoDFrame.cpp
    libs/FujinonSX800Core/OpticalTables.h
    libs/FujinonSX800Core/OpticalTables.cpp
    libs/FujinonSX800Core/ProtocolBuilder.h
    libs/FujinonSX800Core/ProtocolBuilder.cpp
    libs/FujinonSX800Core/ProtocolParser.h
    libs/FujinonSX800Core/ProtocolParser.cpp
    libs/FujinonSX800Core/SerialTransport.h
    libs/FujinonSX800Core/SerialTransport.cpp
    libs/FujinonSX800Core/TcpTransport.h
    libs/FujinonSX800Core/TcpTransport.cpp
    libs/FujinonSX800Core/MockCameraDevice.h
    libs/FujinonSX800Core/MockCameraDevice.cpp
    libs/FujinonSX800Core/CircularByteRing.h
    libs/FujinonSX800Core/FujinonCamera.h
    libs/FujinonSX800Core/FujinonCamera.cpp
)

target_include_directories(FujinonSX800Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/FujinonSX800Core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(FujinonSX800Core PUBLIC
    Threads::Threads
    glog::glog
)
if(WIN32)
    target_link_libraries(FujinonSX800Core PUBLIC ws2_32)
endif()

# ==============================================================================
# 2. Qt Adapter Library
# ==============================================================================
add_library(FujinonSX800Qt STATIC
    libs/FujinonSX800Qt/QFujinonCamera.h
    libs/FujinonSX800Qt/QFujinonCamera.cpp
)

target_include_directories(FujinonSX800Qt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/FujinonSX800Qt>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(FujinonSX800Qt PUBLIC
    FujinonSX800Core
    Qt6::Core
)

# ==============================================================================
# 3. GUI Application
# ==============================================================================
add_executable(FujinonSX800App
    app-qt/main.cpp
    app-qt/MainWindow.h
    app-qt/MainWindow.cpp
    app-qt/widgets/ConnectionWidget.h
    app-qt/widgets/ConnectionWidget.cpp
    app-qt/widgets/TrafficInspectorWidget.h
    app-qt/widgets/TrafficInspectorWidget.cpp
    app-qt/tabs/OpticsTab.h
    app-qt/tabs/OpticsTab.cpp
    app-qt/tabs/ImageQualityTab.h
    app-qt/tabs/ImageQualityTab.cpp
    app-qt/tabs/DayNightTab.h
    app-qt/tabs/DayNightTab.cpp
    app-qt/tabs/OsdVideoTab.h
    app-qt/tabs/OsdVideoTab.cpp
    app-qt/tabs/SdCardMenuTab.h
    app-qt/tabs/SdCardMenuTab.cpp
    app-qt/tabs/SystemTab.h
    app-qt/tabs/SystemTab.cpp
    app-qt/resources.qrc
)

target_include_directories(FujinonSX800App PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app-qt
)

target_link_libraries(FujinonSX800App PRIVATE
    FujinonSX800Qt
    Qt6::Widgets
)

# ==============================================================================
# 4. Automated Unit Tests
# ==============================================================================
enable_testing()

function(add_sx800_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE FujinonSX800Core)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_sx800_test(TestPelcoDFrame tests/TestPelcoDFrame.cpp)
add_sx800_test(TestProtocolBuilder tests/TestProtocolBuilder.cpp)
add_sx800_test(TestProtocolParser tests/TestProtocolParser.cpp)
add_sx800_test(TestOpticalTables tests/TestOpticalTables.cpp)
add_sx800_test(TestMockCamera tests/TestMockCamera.cpp)
add_sx800_test(TestCircularByteRing tests/TestCircularByteRing.cpp)
