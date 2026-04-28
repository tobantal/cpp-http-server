cmake_minimum_required(VERSION 3.14)
project(cpp-http-server VERSION 0.3.2 LANGUAGES CXX)

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
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  cpp-http-server ${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Fetch deps:      ${CPP_HTTP_SERVER_FETCH_DEPS}")
message(STATUS "========================================")
message(STATUS "")

# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(microservice-core)
add_subdirectory(microservice-boost)

enable_testing()

add_subdirectory(microservice-core/tests)
add_subdirectory(microservice-boost/tests)