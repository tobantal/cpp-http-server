# Tests for microservice-core
# Author: Anton Tobolkin

cmake_minimum_required(VERSION 3.14)

# Enable testing
enable_testing()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(microservice-boost-test
    BeastRequestAdapterTest.cpp
    BeastResponseAdapterTest.cpp
    ServerSettingsTest.cpp
    DbSettingsTest.cpp
    HttpClientTest.cpp
)

target_link_libraries(microservice-boost-test
    microservice-boost
    gtest
    gtest_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(microservice-boost-test)