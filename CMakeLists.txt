cmake_minimum_required(VERSION 3.14)
project(cpp-http-server VERSION 0.4.0 LANGUAGES CXX)

set(PROJECT_VERSION_SUFFIX "-beta" CACHE STRING "Version suffix (e.g. -beta, -rc1, empty for release)")

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# ============================================================================
# C++ standard
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Option: auto-fetch dependencies via FetchContent when not found
# ============================================================================
option(CPP_HTTP_SERVER_FETCH_DEPS "Fetch missing dependencies via FetchContent" ON)

# ============================================================================
# Boost
# ============================================================================
if(CPP_HTTP_SERVER_FETCH_DEPS)
    find_package(Boost 1.70 QUIET COMPONENTS system)
endif()

if(Boost_FOUND)
    message(STATUS "Using system Boost ${Boost_VERSION}")
    find_package(Boost REQUIRED COMPONENTS system)
    find_package(Boost REQUIRED COMPONENTS beast asio)
else()
    message(STATUS "Boost not found, fetching via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0.tar.gz
        URL_HASH SHA256=0c6049764e80aa32754acd7d4f179fd5551d8172a83b71532ae093e7384e98da
    )
    set(BOOST_INCLUDE_LIBRARIES system beast asio)
    set(BOOST_ENABLE_CMAKE ON)
    FetchContent_MakeAvailable(Boost)
    message(STATUS "Boost fetched via FetchContent")
endif()

# ============================================================================
# nlohmann/json
# ============================================================================
if(CPP_HTTP_SERVER_FETCH_DEPS)
    find_package(nlohmann_json 3.9 QUIET)
endif()

if(nlohmann_json_FOUND)
    message(STATUS "Using system nlohmann_json ${nlohmann_json_VERSION}")
else()
    message(STATUS "nlohmann_json not found, fetching via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
    message(STATUS "nlohmann_json fetched via FetchContent")
endif()

# ============================================================================
# AMQP-CPP (RabbitMQ client library)
# ============================================================================
if(CPP_HTTP_SERVER_FETCH_DEPS)
    find_package(amqpcpp QUIET)
endif()

if(amqpcpp_FOUND)
    message(STATUS "Using system amqpcpp ${amqpcpp_VERSION}")
else()
    message(STATUS "amqpcpp not found, fetching via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        amqpcpp
        GIT_REPOSITORY https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
        GIT_TAG v4.3.26
        GIT_SHALLOW TRUE
    )
    set(AMQP-CPP_LINUX_TCP ON CACHE BOOL "Enable TCP support in AMQP-CPP" FORCE)
    FetchContent_MakeAvailable(amqpcpp)
    message(STATUS "amqpcpp fetched via FetchContent")
endif()

# ============================================================================
# GoogleTest
# ============================================================================
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

# ============================================================================
# libpqxx
# ============================================================================
if(CPP_HTTP_SERVER_FETCH_DEPS)
    find_package(pqxx 7.0 QUIET)
endif()

if(pqxx_FOUND)
    message(STATUS "Using system pqxx ${pqxx_VERSION}")
else()
    message(STATUS "pqxx not found, fetching via FetchContent...")
    find_package(PostgreSQL REQUIRED)
    set(PQXX_BUILD_TEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        pqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG 7.9.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(pqxx)
    message(STATUS "pqxx fetched via FetchContent")
endif()

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  cpp-http-server ${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX}")
message(STATUS "========================================")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Fetch deps:      ${CPP_HTTP_SERVER_FETCH_DEPS}")
message(STATUS "========================================")
message(STATUS "")

# ============================================================================
# CMake modules (for external projects using FetchContent)
# ============================================================================
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddHttpService.cmake)

# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(microservice-core)
add_subdirectory(microservice-boost)

enable_testing()

add_subdirectory(microservice-core/tests)
add_subdirectory(microservice-boost/tests)