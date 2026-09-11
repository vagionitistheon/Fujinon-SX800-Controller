# ==============================================================================
# Qt Adapter Library
# ==============================================================================
add_library(FujinonSX800Qt STATIC
    QFujinonCamera.h
    QFujinonCamera.cpp
)

target_include_directories(FujinonSX800Qt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(FujinonSX800Qt PUBLIC
    FujinonSX800Core
    Qt6::Core
)

apply_compiler_flags(FujinonSX800Qt)

