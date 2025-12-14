# Microservices Project - Root CMakeLists.txt
# Author: Anton Tobolkin
#
# Структура проекта:
# 1. microservice-core - библиотека интерфейсов (header-only)
# 2. microservice-boost - библиотека реализации на Boost

cmake_minimum_required(VERSION 3.14)
project(MicroservicesProject VERSION 1.0.0 LANGUAGES CXX)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем более строгие предупреждения
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Выводим информацию о конфигурации
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Microservices Project Configuration  ")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "========================================")
message(STATUS "")

# ============================================================================
# Автоматическое скачивание Boost (только нужные компоненты)
# ============================================================================
include(FetchContent)

message(STATUS "Fetching Boost from GitHub (this may take a few minutes on first run)...")

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0.tar.gz
    URL_HASH SHA256=0c6049764e80aa32754acd7d4f179fd5551d8172a83b71532ae093e7384e98da
)


# Настройки Boost - собираем только system
set(BOOST_INCLUDE_LIBRARIES system beast asio)
set(BOOST_ENABLE_CMAKE ON)

FetchContent_MakeAvailable(Boost)

message(STATUS "Boost successfully fetched and configured")

# ============================================================================
# Добавляем Boost.DI (header-only)
# ============================================================================
message(STATUS "Fetching Boost.DI from GitHub...")

FetchContent_Declare(
    BoostDI
    GIT_REPOSITORY https://github.com/boost-ext/di.git
    GIT_TAG v1.3.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(BoostDI)

message(STATUS "Boost.DI successfully fetched")

# ============================================================================
# Добавляем nlohmann/json (header-only)
# ============================================================================
message(STATUS "Fetching nlohmann/json from GitHub...")

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

message(STATUS "nlohmann/json successfully fetched")

# Добавляем подпроекты в порядке зависимостей
message(STATUS "Adding microservice-core...")
add_subdirectory(microservice-core)
add_subdirectory(microservice-core/tests)

message(STATUS "Adding microservice-boost...")
add_subdirectory(microservice-boost)
add_subdirectory(microservice-boost/tests)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Configuration completed successfully ")
message(STATUS "========================================")
message(STATUS "")
