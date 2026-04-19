add_executable(microservice-core-test
    RouteMatcherTest.cpp
    ThreadSafeMapTest.cpp
    EnvironmentTest.cpp
    SimpleRequestTest.cpp
    SimpleResponseTest.cpp
    HttpErrorTest.cpp
    ChainHandlerTest.cpp
    StringUtilsTest.cpp
)

target_link_libraries(microservice-core-test
    microservice-core
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(microservice-core-test)