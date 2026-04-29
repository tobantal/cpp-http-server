cmake_minimum_required(VERSION 3.14)

add_library(microservice-boost
    src/BoostBeastApplication.cpp
    src/HttpClient.cpp
    src/BeastRequestAdapter.cpp
    src/BeastResponseAdapter.cpp
    src/ServerSettings.cpp
    src/JsonValidator.cpp
)

target_include_directories(microservice-boost
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)

target_link_libraries(microservice-boost
    PUBLIC
        microservice-core
        Boost::system
        Boost::beast
        Boost::asio
        nlohmann_json::nlohmann_json
        Threads::Threads
)

target_compile_features(microservice-boost PUBLIC cxx_std_17)
