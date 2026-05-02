add_executable(microservice-boost-test
    BeastRequestAdapterTest.cpp
    BeastResponseAdapterTest.cpp
    ConnectionPoolTest.cpp
    DbSettingsTest.cpp
    HttpClientTest.cpp
    JsonProcessorTest.cpp
    JsonSerializerTest.cpp
    JsonToEnvConverterTest.cpp
    JsonValidatorTest.cpp
    PostgresHealthCheckTest.cpp
    RabbitMQHealthCheckTest.cpp
    SchemaValidatorTest.cpp
    ServerSettingsTest.cpp
    SplunkLoggerTest.cpp
)

target_link_libraries(microservice-boost-test
    PRIVATE
        microservice-boost
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(microservice-boost-test)