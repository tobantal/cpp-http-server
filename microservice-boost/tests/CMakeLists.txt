add_executable(microservice-boost-test
    BeastRequestAdapterTest.cpp
    BeastResponseAdapterTest.cpp
    JsonValidatorTest.cpp
    ServerSettingsTest.cpp
    ServerStateTest.cpp
    HttpClientTest.cpp
    JsonSerializerTest.cpp
)

target_link_libraries(microservice-boost-test
    PRIVATE
        microservice-boost
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(microservice-boost-test)