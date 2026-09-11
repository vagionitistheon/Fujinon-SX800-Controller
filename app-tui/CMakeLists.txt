# ==============================================================================
# Pure C++17 Terminal User Interface (Zero-dependency beyond OS console/termios)
# ==============================================================================
add_executable(FujinonSX800Tui
    main.cpp
    Terminal.h
    Terminal.cpp
    Color.h
    Canvas.h
    Canvas.cpp
    UtfSymbols.h
    TuiApp.h
    TuiApp.cpp
    views/HeaderView.h
    views/HeaderView.cpp
    views/OpticsView.h
    views/OpticsView.cpp
    views/ImageQualityView.h
    views/ImageQualityView.cpp
    views/DayNightView.h
    views/DayNightView.cpp
    views/OsdVideoView.h
    views/OsdVideoView.cpp
    views/SdCardView.h
    views/SdCardView.cpp
    views/SystemView.h
    views/SystemView.cpp
    views/TrafficView.h
    views/TrafficView.cpp
    views/FooterView.h
    views/FooterView.cpp
    views/ConnectionModal.h
    views/ConnectionModal.cpp
)

target_include_directories(FujinonSX800Tui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs
)

target_link_libraries(FujinonSX800Tui PRIVATE
    FujinonSX800Core
)

apply_compiler_flags(FujinonSX800Tui)
