# Microservice Boost Library - Реализация на Boost.DI и Boost.Beast
# Author: Anton Tobolkin

cmake_minimum_required(VERSION 3.14)

# Создаем библиотеку с реализацией
add_library(microservice-boost
    src/BoostBeastApplication.cpp
    src/settings/DbSettings.cpp
    src/HttpClient.cpp
)

# Подключаем заголовки
target_include_directories(microservice-boost PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Линкуем с зависимостями
target_link_libraries(microservice-boost PUBLIC
    microservice-core
    Boost::system
    Boost::beast
    Boost::asio
    nlohmann_json::nlohmann_json
    pthread
)

# Требуем C++17
target_compile_features(microservice-boost PUBLIC cxx_std_17)