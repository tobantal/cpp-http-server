add_executable(microservice-boost-test
    BeastRequestAdapterTest.cpp
    BeastResponseAdapterTest.cpp
    DbSettingsTest.cpp
    JsonValidatorTest.cpp
    ServerSettingsTest.cpp
    HttpClientTest.cpp
    SplunkLoggerTest.cpp
    JsonSerializerTest.cpp
    JsonToEnvConverterTest.cpp
    JsonProcessorTest.cpp
    PostgresTransactionExecutorTest.cpp
)

target_link_libraries(microservice-boost-test
    PRIVATE
        microservice-boost
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(microservice-boost-test)