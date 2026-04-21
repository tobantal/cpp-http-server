# Microservice Core Library - Interfaces and utilities (Hexagonal Architecture)
# Author: Anton Tobolkin

cmake_minimum_required(VERSION 3.14)

# Generate version.hpp from PROJECT_VERSION (set in root CMakeLists.txt)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp
    @ONLY
)

add_library(microservice-core
    src/ChainHandler.cpp
    src/ConsoleLogger.cpp
    src/LogLevel.cpp
    src/MetricsCollector.cpp
    src/MetricsHandler.cpp
    src/MetricsObserverHandler.cpp
    src/NullLogger.cpp
    src/RouteMatcher.cpp
    src/Timer.cpp
    src/TestLogger.cpp
)

target_include_directories(microservice-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_features(microservice-core PUBLIC cxx_std_17)
