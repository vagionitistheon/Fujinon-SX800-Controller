# ==============================================================================
# Pure C++17 Core Library (Zero Qt dependency)
# ==============================================================================
add_library(FujinonSX800Core STATIC
    PelcoDTypes.h
    CameraStatus.h
    ITransport.h
    PelcoDFrame.h
    PelcoDFrame.cpp
    OpticalTables.h
    OpticalTables.cpp
    ProtocolBuilder.h
    ProtocolBuilder.cpp
    ProtocolParser.h
    ProtocolParser.cpp
    SerialTransport.h
    SerialTransport.cpp
    TcpTransport.h
    TcpTransport.cpp
    MockCameraDevice.h
    MockCameraDevice.cpp
    CircularByteRing.h
    FujinonCamera.h
    FujinonCamera.cpp
)

target_include_directories(FujinonSX800Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(FujinonSX800Core PUBLIC
    Threads::Threads
    glog::glog
)

if(WIN32)
    target_link_libraries(FujinonSX800Core PUBLIC ws2_32)
endif()

apply_compiler_flags(FujinonSX800Core)
