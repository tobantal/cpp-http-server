cmake_minimum_required(VERSION 3.14)
project(MicroservicesProject VERSION 1.0.0 LANGUAGES CXX)

# Enable Google Tests
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Microservices Project Configuration  ")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "========================================")
message(STATUS "")

include(FetchContent)

# ============================================================================
# Boost
# ============================================================================

message(STATUS "Fetching Boost from GitHub...")

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0.tar.gz
    URL_HASH SHA256=0c6049764e80aa32754acd7d4f179fd5551d8172a83b71532ae093e7384e98da
)

set(BOOST_INCLUDE_LIBRARIES system beast asio)
set(BOOST_ENABLE_CMAKE ON)

FetchContent_MakeAvailable(Boost)

message(STATUS "Boost successfully fetched")

# ============================================================================
# Boost.DI
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
# nlohmann/json
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

# ============================================================================
# microservice-core
# ============================================================================

message(STATUS "Adding microservice-core...")

add_subdirectory(microservice-core)

# ============================================================================
# microservice-boost
# ============================================================================

message(STATUS "Adding microservice-boost...")

add_subdirectory(microservice-boost)

# ============================================================================
# Tests (optional)
# ============================================================================

add_subdirectory(microservice-core/tests)
add_subdirectory(microservice-boost/tests)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Configuration completed successfully ")
message(STATUS "========================================")
message(STATUS "")
