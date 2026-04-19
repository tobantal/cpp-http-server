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
- Исключение: шаблоны (template) остаются header-only, но реализация в блоке `#include "*.impl.hpp"` или в конце файла
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