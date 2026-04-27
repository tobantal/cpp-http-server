cmake_minimum_required(VERSION 3.14)

add_library(microservice-boost
    src/BoostBeastApplication.cpp
    src/ConnectionPool.cpp
    src/HttpClient.cpp
    src/PostgresTransactionExecutor.cpp
)

target_include_directories(microservice-boost
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)

target_link_libraries(microservice-boost
    PUBLIC
        microservice-core
        Boost::system
        Boost::beast
        Boost::asio
        nlohmann_json::nlohmann_json
        libpqxx::pqxx
        PostgreSQL::PostgreSQL
        Threads::Threads
)

target_compile_features(microservice-boost PUBLIC cxx_std_17)
