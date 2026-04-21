# Architecture — структура файлов проекта

## Двухмодульная структура

```
cpp-http-server/
├── microservice-core/          # Интерфейсы и утилиты — нулевые зависимости
│   ├── include/
│   │   ├── IRequest.hpp        # Интерфейсы (I-префикс) — в корне
│   │   ├── IResponse.hpp
│   │   ├── IHttpHandler.hpp
│   │   ├── IWebApplication.hpp
│   │   ├── IHttpClient.hpp
│   │   ├── IEnvironment.hpp
│   │   ├── HttpStatus.hpp      # Общие enum/struct — в корне
│   │   ├── error/             # Классы ошибок
│   │   │   ├── HttpError.hpp
│   │   │   ├── NotFoundError.hpp
│   │   │   ├── BadRequestError.hpp
│   │   │   └── ...
│   │   ├── handler/           # Хендлеры и middleware
│   │   │   ├── ChainHandler.hpp
│   │   │   ├── HealthHandler.hpp
│   │   │   └── ...
│   │   ├── util/              # Утилиты
│   │   │   ├── StringUtils.hpp
│   │   │   ├── ThreadSafeMap.hpp
│   │   │   ├── PathParamExtractor.hpp
│   │   │   └── ...
│   │   ├── settings/          # Настройки и конфигурация
│   │   │   └── ...
│   │   └── SimpleRequest.hpp  # Simple-реализации — в корне (тестовый double)
│   ├── src/                   # .cpp реализации (RouteMatcher.cpp)
│   └── tests/
├── microservice-boost/        # Boost-реализация
│   ├── include/
│   │   ├── BoostBeastApplication.hpp
│   │   ├── BeastRequestAdapter.hpp
│   │   ├── BeastResponseAdapter.hpp
│   │   ├── HttpClient.hpp
│   │   ├── handler/
│   │   │   └── JsonValidator.hpp
│   │   └── settings/
│   │       └── ServerSettings.hpp
│   ├── src/
│   └── tests/
```

## Правило: интерфейсы vs реализации vs ошибки

### Интерфейсы (I-префикс)
Файлы: `IRequest.hpp`, `IResponse.hpp`, `IHttpHandler.hpp`, `IWebApplication.hpp`, `IHttpClient.hpp`, `IEnvironment.hpp`

**Структура интерфейса:**
- Только `virtual` чистые методы + виртуальный деструктор
- **Без** переменных-членов (кроме виртуального деструктора)
- **Без** реализаций (кроме `= default` деструктор)
- **Без** `#include` сторонних библиотек в core-интерфейсах
- Документация через `/** */` комментарии

### Заголовки классов (*.hpp)
- Объявление класса + документация (`/** */`) — **без** реализаций методов
- Все методы (кроме виртуальных интерфейсов `= 0`) — реализация в `.cpp`
- **Исключение: шаблоны (template)** — template-методы и классы с template-параметрами обязаны оставаться в `.hpp`, их невозможно инстанцировать в `.cpp` без явной специализации. Реализация: `#include "*.impl.hpp"` в конце `.hpp` или прямо в классе.
- 1 класс = 1 файл

### Доменные подпапки
- `error/` — все классы ошибок (HttpError, NotFoundError, ...)
- `handler/` — хендлеры и middleware (ChainHandler, HealthHandler, JsonValidator)
- `util/` — утилиты (StringUtils, ThreadSafeMap, PathParamExtractor)
- `settings/` — конфигурация (ServerSettings, IEnvironment, Environment)
- Корень `include/` — только интерфейсы (I-префикс), HttpStatus, SimpleRequest/SimpleResponse

### Backward compatibility
При перемещении файла в подпапку — создать forwarding-заголовок в корне:
```cpp
// include/NotFoundError.hpp — DEPRECATED, use error/NotFoundError.hpp
#pragma once
#include "error/NotFoundError.hpp"
```
Удалить forwarding-заголовки в следующей major-версии.

## Правило зависимостей
- microservice-core **НЕ** зависит от Boost, nlohmann/json, или любой сторонней библиотеки
- microservice-boost зависит от microservice-core + Boost + nlohmann/json
- Consumer-проекты зависят только от нужных им модулей

## Правило: SOLID
Все интерфейсы и классы библиотеки следуют принципам SOLID:

- **S** (Single Responsibility): один класс — одна ответственность
- **O** (Open/Closed): расширение через интерфейсы, без модификации существующего кода
- **L** (Liskov Substitution): реализации интерфейсов взаимозаменяемы
- **I** (Interface Segregation): маленькие специализированные интерфейсы вместо одного большого. Общие методы выделяются в отдельные интерфейсы (например, `INameable` для `name()`), которые наследуются узкими интерфейсами
- **D** (Dependency Inversion): зависимость от абстракций (интерфейсов), не от реализаций

## Правило: замер времени
Для замера времени выполнения использовать `Timer` (`util/Timer.hpp`), а не ручной `std::chrono::steady_clock::now()`. API: `start()`, `stop()`, `elapsed(TimeUnit)`, `show(TimeUnit)`.

## Правило: ENV-переменные
При добавлении новой ENV-переменной — сразу обновлять `.env.example` и `docs/configuration.md`. Каждая настройка: название, тип, значение по умолчанию, описание.