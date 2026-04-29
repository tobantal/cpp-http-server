# Getting Started

Step-by-step guide to create your first HTTP service with cpp-http-server.

---

## 1. Create Project

```bash
mkdir my-service && cd my-service
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_service)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    cpp-http-server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG v0.4.0
)

FetchContent_MakeAvailable(cpp-http-server)

add_executable(my_service src/main.cpp)
target_link_libraries(my_service microservice-boost)
```

---

## 2. Create Application Class

Create `include/MyWebApp.hpp`:

```cpp
#pragma once

#include "BoostBeastApplication.hpp"
#include "HealthHandler.hpp"

class MyWebApp : public BoostBeastApplication {
public:
    void configureInjection() override {
        // Register handlers
        registerEndpoint("GET", "/health", std::make_shared<HealthHandler>());
    }
};
```

---

## 3. Create main.cpp

Create `src/main.cpp`:

```cpp
#include "MyWebApp.hpp"

int main(int argc, char* argv[]) {
    MyWebApp app;
    return app.run(argc, argv);
}
```

---

## 4. Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

---

## 5. Run

```bash
./my_service
# Server starts on http://0.0.0.0:8080
```

Test:
```bash
curl http://localhost:8080/health
# {"status":"UP"}
```

---

## Configuration

Default settings can be overridden via ENV variables or `config.json`.

### ENV Variables

```bash
export SERVER_PORT=9090
export SERVER_HOST=127.0.0.1
./my_service
```

### config.json

Create `config.json`:

```json
{
  "server": {
    "host": "127.0.0.1",
    "port": 9090
  }
}
```

Pass path to application:
```cpp
int main(int argc, char* argv[]) {
    MyWebApp app;
    app.loadConfig("config.json");
    return app.run(argc, argv);
}
```

See [docs/configuration.md](configuration.md) for all settings.

---

## Next Steps

- [Architecture Overview](architecture.md) — understand the design
- [API Reference](api/) — detailed interface documentation
- [Development Guidelines](development/) — coding standards