cmake_minimum_required(VERSION 3.16)
project(FujinonSX800Controller VERSION 2.12.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags, security hardening, sanitizers, and warnings
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerFlags)
include(GitHooks)

find_package(Threads REQUIRED)
find_package(glog REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

# Option to toggle unit tests
option(BUILD_TESTING "Build automated unit tests" ON)

# Subdirectories
add_subdirectory(libs)
add_subdirectory(app-qt)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
