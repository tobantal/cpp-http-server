# Architecture — структура и принципы проектирования

## Двухмодульная структура

```
cpp-http-server/
├── microservice-core/          # Интерфейсы и утилиты — нулевые зависимости
│   ├── include/
│   │   ├── domain/             # Доменные типы (IRequest, IResponse, HttpError)
│   │   ├── error/              # Классы ошибок (NotFoundError, BadRequestError, ...)
│   │   ├── ports/input/        # Входные порты (IHttpHandler, IWebApplication)
│   │   ├── ports/output/       # Выходные порты (ILogger, IHttpClient, IIdGenerator)
│   │   ├── application/        # Application-level (ChainHandler, ShutdownManager)
│   │   ├── handler/            # Хендлеры (HealthHandler, MetricsHandler)
│   │   ├── metrics/            # Метрики (MetricsCollector)
│   │   ├── util/               # Утилиты (StringUtils, Timer, UuidGenerator)
│   │   └── adapters/          # Адаптеры (SimpleRequest, SimpleResponse)
│   └── src/                   # .cpp реализации (RouteMatcher.cpp)
├── microservice-boost/        # Boost-реализация
│   ├── include/
│   │   ├── adapters/primary/   # BeastRequestAdapter, BeastResponseAdapter
│   │   ├── adapters/secondary/ # HttpClient, ConnectionPool
│   │   └── settings/           # ServerSettings
│   └── src/
```

---

## Hexagonal Architecture (Ports and Adapters)

### Концепция

```
                    ┌─────────────────────────────┐
                    │       Consumer App          │
                    │   (trading-platform, etc.)  │
                    └──────────────┬──────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │      microservice-boost     │
                    │   (Driving Adapters)        │
                    │   BoostBeastApplication     │
                    │   BeastRequestAdapter       │
                    └──────────────┬──────────────┘
                                   │
              ┌────────────────────▼────────────────────┐
              │           microservice-core               │
              │  ┌─────────────────────────────────────┐ │
              │  │          Ports (Interfaces)         │ │
              │  │  input: IHttpHandler, IWebApplication│ │
              │  │  output: ILogger, IHttpClient       │ │
              │  └─────────────────────────────────────┘ │
              │  ┌─────────────────────────────────────┐ │
              │  │     Domain / Application Logic      │ │
              │  │  ChainHandler, HealthHandler,      │ │
              │  │  MetricsHandler, RouteMatcher      │ │
              │  └─────────────────────────────────────┘ │
              └────────────────────┬────────────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │   (Driven Adapters)          │
                    │   External systems:           │
                    │   Network, Config, etc.       │
                    └─────────────────────────────┘
```

### Ports (интерфейсы)

**Input Ports** — определяют как внешний мир взаимодействует с приложением:
- `IHttpHandler` — обработчик HTTP-запросов
- `IWebApplication` — жизненный цикл приложения

**Output Ports** — определяют как приложение взаимодействует с внешним миром:
- `ILogger` — логирование
- `IHttpClient` — исходящие HTTP-запросы
- `IIdGenerator` — генерация ID
- `IEnvironment` — конфигурация

### Adapters (реализации)

**Driving Adapters** — преобразуют внешние стимулы в вызовы input ports:
- `BoostBeastApplication` — HTTP-сервер → вызывает `IWebApplication::run()`
- `BeastRequestAdapter` — Boost.Beast request → `IRequest`

**Driven Adapters** — реализуют output ports для внешних систем:
- `HttpClient` — реализует `IHttpClient` через Boost.Asio
- `ConsoleLogger` — реализует `ILogger`

### Правило зависимостей

**Зависимость направлена только внутрь.** Внутренние слои не знают о внешних.

- `microservice-core` НЕ зависит от `microservice-boost` (чистая библиотека)
- `microservice-boost` зависит от `microservice-core`
- Consumer-проекты зависят от нужных им модулей

---

## SOLID Principles

### S — Single Responsibility Principle

**Один класс — одна причина для изменения.**

```cpp
// ПРАВИЛЬНО: один класс — одна ответственность
class HealthHandler : public IHttpHandler {
    void handle(IRequest& req, IResponse& res) override {
        res.setResult(200, "application/json", R"({"status":"UP"})");
    }
    std::string name() const override { return "HealthHandler"; }
};

// ПРАВИЛЬНО: ChainHandler только объединяет middleware
class ChainHandler : public IHttpHandler {
    // Не логирует, не валидирует — только координирует
};

// НЕПРАВИЛЬНО: смешивание ответственностей
class BigHandler : public IHttpHandler {
    void handle(IRequest& req, IResponse& res) override {
        // Разбор параметров
        // Валидация
        // Бизнес-логика
        // Логирование
        // Форматирование ответа
    }
};
```

### O — Open/Closed Principle

**Открыто для расширения, закрыто для модификации.**

```cpp
// ПРАВИЛЬНО: расширение через наследование/композицию
class MetricsObserverHandler : public IHttpHandler {
    // Добавляет метрики, не изменяя original handler
    std::shared_ptr<IHttpHandler> inner_;
    std::shared_ptr<IMetricsCollector> metrics_;
};

// НЕПРАВИЛЬНО: добавление new if/else в существующий код
class BadChainHandler {
    void handle(IRequest& req, IResponse& res) override {
        if (someCondition) { /* new behavior */ }
        // existing code...
    }
};
```

### L — Liskov Substitution Principle

**Объекты базового класса заменяемы объектами производного.**

```cpp
// ПРАВИЛЬНО: реализации взаимозаменяемы
class UuidGenerator : public IIdGenerator {
    std::string generate() override { /* ... */ }
};

// В тестах можно подменить на mock
class MockIdGenerator : public IIdGenerator {
    std::string generate() override { return "mock-id"; }
};

// Потребитель не знает какая реализация
void createUser(std::shared_ptr<IIdGenerator> idGen) {
    auto id = idGen->generate(); // работает с любой реализацией
}
```

### I — Interface Segregation Principle

**Много специализированных интерфейсов лучше чем один универсальный.**

```cpp
// ПРАВИЛЬНО: маленькие интерфейсы
class INameable {
public:
    virtual ~INameable() = default;
    virtual std::string name() const = 0;
};

class IHttpHandler : public INameable {
public:
    virtual void handle(IRequest& req, IResponse& res) = 0;
    // name() уже есть от INameable
};

// НЕПРАВИЛЬНО: один большой интерфейс
class IBigHandler {
    virtual void handle();
    virtual std::string name();
    virtual void log();
    virtual void validate();
    // ... 20 методов
};
```

### D — Dependency Inversion Principle

**Зависимость от абстракций, не от конкретных реализаций.**

```cpp
// ПРАВИЛЬНО: зависимость от интерфейса
class ChainHandler {
    std::shared_ptr<IHttpErrorHandler> errorHandler_; // абстракция
    std::shared_ptr<ILogger> logger_;                 // абстракция
};

// НЕПРАВИЛЬНО: зависимость от конкретной реализации
class BadChainHandler {
    HttpErrorSender errorHandler_; // конкретный класс
    ConsoleLogger logger_;         // конкретный класс
};
```

---

## GoF Patterns Used

### Adapter Pattern

**Преобразует интерфейс одного класса в интерфейс другого.**

```cpp
// BeastRequestAdapter: Beast HTTP request → IRequest
class BeastRequestAdapter : public IRequest {
    // адаптирует boost::beast::http::request к IRequest
};

// BeastResponseAdapter: IResponse → Beast HTTP response
class BeastResponseAdapter : public /* Beast response */ {
    // адаптирует IResponse к boost::beast::http::response
};
```

### Chain of Responsibility Pattern

**Обработчики связаны в цепочку, запрос проходит пока не будет обработан.**

```cpp
class ChainHandler {
    std::vector<std::shared_ptr<IHttpHandler>> handlers_;
    // каждый handler вызывает следующий или возвращает ответ
};
```

### Template Method Pattern

**Определяет скелет алгоритма, подклассы определяют конкретные шаги.**

```cpp
class IWebApplication {
    int run(int argc, char* argv[]) {
        loadEnvironment();    // шаг 1
        configureInjection();  // шаг 2 — override в подклассе
        start();              // шаг 3
    }
};
```

### Decorator Pattern

**Добавляет поведение динамически, оборачивая объект.**

```cpp
// MetricsObserverHandler оборачивает handler и добавляет метрики
class MetricsObserverHandler : public IHttpHandler {
    std::shared_ptr<IHttpHandler> inner_; // оборачиваемый handler
    std::shared_ptr<IMetricsCollector> metrics_;
    // добавляет метрики до/после вызова inner
};
```

### Strategy Pattern

**Инкапсулирует алгоритм, делает взаимозаменяемым.**

```cpp
// IIdGenerator — стратегия генерации ID
// UuidGenerator — конкретная стратегия
// В тестах: MockIdGenerator — тестовая стратегия

// IHttpErrorHandler — стратегия обработки ошибок
// HttpErrorSender — конкретная стратегия
// В user code: XmlErrorHandler — альтернативная стратегия
```

---

## Правила организации кода

### Интерфейсы (*.hpp в ports/)

- Только `virtual` чистые методы + виртуальный деструктор
- **Без** переменных-членов
- **Без** реализаций методов (кроме `= default` деструктор)
- **Без** `#include` сторонних библиотек в core-интерфейсах

### Реализации

- Объявление класса + документация в `.hpp`
- Реализация методов в `.cpp`
- **Исключение: template-методы** — обязаны оставаться в `.hpp`

### 1 класс = 1 файл

### Backward Compatibility

При перемещении файла в подпапку — создать forwarding-заголовок:
```cpp
// include/NotFoundError.hpp — DEPRECATED, use error/NotFoundError.hpp
#pragma once
#include "error/NotFoundError.hpp"
```

---

## Правило: замер времени

Использовать `Timer` (`util/Timer.hpp`), а не `std::chrono::steady_clock::now()` напрямую.

---

## Правило: ENV-переменные

При добавлении новой ENV-переменной — обновить `.env.example` и `docs/configuration.md`.

---

## Критерии качества

При каждом изменении проверять:

1. **Separation core/boost** — core не зависит от boost
2. **Hexagonal structure** — порты и адаптеры разделены
3. **SOLID compliance** — ни один принцип не нарушен
4. **Backward compatibility** — consumer проект собирается
5. **Thread-safety** — параллельный доступ без гонок
6. **Error handling** — все пути ведут к корректному ответу