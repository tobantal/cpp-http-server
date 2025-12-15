# Tests for microservice-boost

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(microservice-boost-test
    BeastRequestAdapterTest.cpp
    BeastResponseAdapterTest.cpp
    ServerSettingsTest.cpp
    DbSettingsTest.cpp
    HttpClientTest.cpp
)

target_link_libraries(microservice-boost-test
    PRIVATE
        microservice-boost
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(microservice-boost-test)
