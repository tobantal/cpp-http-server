# Microservice Core Library - Interfaces and utilities (Hexagonal Architecture)
# Author: Anton Tobolkin

cmake_minimum_required(VERSION 3.14)

add_library(microservice-core
    src/ChainHandler.cpp
    src/ConsoleLogger.cpp
    src/LogLevel.cpp
    src/NullLogger.cpp
    src/RouteMatcher.cpp
    src/TestLogger.cpp
)

target_include_directories(microservice-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(microservice-core PUBLIC cxx_std_17)
