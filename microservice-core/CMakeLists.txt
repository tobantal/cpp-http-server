# Microservice Core Library - Интерфейсы и утилиты
# Author: Anton Tobolkin

cmake_minimum_required(VERSION 3.14)

# Создаем библиотеку с реализацией утилит
add_library(microservice-core
    src/ChainHandler.cpp
    src/ConsoleLogger.cpp
    src/LogLevel.cpp
    src/NullLogger.cpp
    src/RouteMatcher.cpp
    src/TestLogger.cpp
)

# Подключаем заголовки
target_include_directories(microservice-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Требуем C++17
target_compile_features(microservice-core PUBLIC cxx_std_17)
