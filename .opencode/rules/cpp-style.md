# C++ Style — правила для cpp-http-server

> Применимо к библиотеке HTTP-сервера на C++17.

## Стандарт и совместимость

- **C++17** (не менять без согласования)
- Компиляторы: GCC 12+, Clang 15+
- CMake 3.14+

## Организация кода

### Структура проекта
- **microservice-core/** — интерфейсы, утилиты, заголовки. Нулевые зависимости.
  - `include/` — публичные заголовки (IRequest, IResponse, IHttpHandler, ...)
  - `src/` — реализации (RouteMatcher.cpp — единственный .cpp в core)
  - `tests/` — unit-тесты
- **microservice-boost/** — реализация на Boost.Beast.
  - `include/` — BoostBeastApplication, BeastRequestAdapter, BeastResponseAdapter, HttpClient, settings/
  - `src/` — BoostBeastApplication.cpp, HttpClient.cpp
  - `tests/` — unit и integration тесты

### Правило зависимостей
- microservice-core **НЕ** зависит от Boost, nlohmann/json, или любой другой сторонней библиотеки
- microservice-boost зависит от microservice-core + Boost + Boost.DI + nlohmann/json
- Consumer-проекты зависят только от нужных им модулей

### Структура файла
- 1 класс = 1 файл (исключение: тесно связанные мелкие классы)
- Header-guards: `MICROSERVICE_CORE_FILE_HPP` / `MICROSERVICE_BOOST_FILE_HPP`
- Порядок секций: `public` → `protected` → `private`
- Порядок методов: конструктор → деструктор → публичные → приватные

### Include-порядок
1. Соответствующий `.hpp` (для `.cpp`)
2. `< >` — системные и сторонние (Boost, nlohmann)
3. `" "` — проектные заголовки
4. Между группами — пустая строка

### Header-only vs hpp/cpp
- **Интерфейсы** (только `virtual` = 0, без реализации) — header-only без ограничения размера (IRequest.hpp — 296 строк — ок, это чистый интерфейс + документация)
- **Классы** — реализация всегда в `.cpp`, `.hpp` содержит только объявление + документацию. Без inline-реализаций.
- **Исключение: шаблоны** (template) —_methods и классы с template-параметрами обязаны оставаться в `.hpp`, их невозможно инстанцировать в `.cpp` без явной специализации. Реализация: `#include "*.impl.hpp"` в конце `.hpp` или прямо в классе.

## Naming conventions

| Элемент | Стиль | Пример |
|---------|-------|--------|
| Класс / Struct | PascalCase | `BoostBeastApplication` |
| Интерфейс | PascalCase с `I` префиксом | `IRequest`, `IHttpHandler` |
| Функция / Метод | camelCase | `getPathParam()` |
| Переменная | camelCase | `queryParams` |
| Приватное поле | camelCase с суффиксом `_` | `handlers_`, `ioContext_` |
| Константа | kPascalCase | `kDefaultPort` |
| Enum value | PascalCase | `HttpMethod::Get` |
| Namespace | snake_case | `microservice_core` |
| Файл | PascalCase | `BeastRequestAdapter.hpp` |
| Макрос / Guard | UPPER_SNAKE | `MICROSERVICE_CORE_IREQUEST_HPP` |

## Управление памятью

- `std::shared_ptr` — для handler-ов (shared ownership через DI)
- `std::unique_ptr` — для сокетов, io_context, acceptor
- Неуправляемые `new`/`delete` запрещены
- RAII — все ресурсы (сокеты, соединения) в RAII-объектах

## Const-correctness

- Параметры-ссылки: `const std::string&` если не модифицируется
- Методы: `const` если не меняет состояние
- Итераторы: `cbegin()`/`cend()` если не модифицируем

## Порядок полей в структурах — минимизация padding

Поля в struct/class упорядочивать **по убыванию размера**: 8 байт → 4 байта → 2 байта → 1 байт. Это устраняет padding-зазоры между полями разного размера и улучшает cache locality.

```cpp
// ПРАВИЛЬНО: крупные поля first, мелкие — в конце
struct Order {
    std::string id;              // 32 байт, align 8
    std::string accountId;       // 32 байт, align 8
    int64_t quantity = 0;        // 8 байт,  align 8
    Money price;                 // 48 байт, align 8
    Money executedPrice;          // 48 байт, align 8
    int64_t executedQuantity = 0; // 8 байт,  align 8
    Timestamp createdAt;         // 8 байт,  align 8
    Timestamp updatedAt;         // 8 байт,  align 8
    OrderDirection direction;    // 1 байт,  align 1  ← enum class : uint8_t
    OrderType type;              // 1 байт,  align 1
    OrderStatus status;          // 1 байт,  align 1
};

// НЕПРАВИЛЬНО: enum между string и int64_t — два padding-зазора
struct OrderOld {
    std::string id;
    std::string accountId;
    OrderDirection direction;    // 1 байт → 7 байт padding перед quantity
    int64_t quantity = 0;        // padding перед этим полем
    ...
    Timestamp createdAt;
    OrderStatus status;          // 1 байт → 7 байт padding перед createdAt
};
```

### enum class: всегда `uint8_t`

По умолчанию `enum class` имеет underlying type `int` (4 байта). Для доменных перечислений с малым количеством значений указывать `uint8_t`:

```cpp
enum class ServerState : uint8_t { NotStarted, Running, Stopped };
enum class LogLevel : uint8_t { Debug, Info, Warn, Error };
```

Экономия: 3 байта на каждое перечисление (4 → 1). В структуре с несколькими enum'ами это устраняет padding.

## Кодогенерация и DRY

- Дублирование `toLower()` и `splitPath()` в 4 файлах — вынести в общий utility (`StringUtils.hpp` в core)
- Дублирование `getPathParam()` логики — вынести в общий helper или mixin

## Doxygen-документирование

Все публичные заголовки (`.hpp`) в `include/` обязаны содержать Doxygen-комментарии.

### Файл-комментарий

Каждый `.hpp` начинается с `@file`, `@brief`, `@author` (или `@version` при необходимости):

```cpp
/**
 * @file ChainHandler.hpp
 * @brief Middleware chain handler
 * @author Anton Tobolkin
 */
```

### Классы и структуры

Каждый класс/struct — `@class` (или `@struct`) + `@brief` + развёрнутое описание если нужно:

```cpp
/**
 * @class ChainHandler
 * @brief Middleware chain — executes handlers sequentially
 *
 * Executes each handler in order of addition. On HttpError,
 * delegates to IHttpErrorHandler. On std::exception — delegates with HttpError(500, ...).
 */
```

### Методы и функции

Публичные и защищённые методы — `@brief`, `@param`, `@return` при необходимости:

```cpp
/**
 * @brief Register HTTP handler for method and path pattern
 * @param method HTTP method (GET, POST, etc.)
 * @param pattern URL pattern, supports * wildcard
 * @param handler Handler instance
 */
void registerHandler(const std::string &method,
                     const std::string &pattern,
                     std::shared_ptr<IHttpHandler> handler);
```

Приватные методы — краткий `@brief` в одну строку:

```cpp
/** @brief Find handler by method and path (exact match first, then wildcard) */
std::optional<HandlerMatch> findHandler(const std::string &method, const std::string &path);
```

### Поля

Публичные и защищённые поля — краткий `@brief`:

```cpp
std::shared_ptr<ILogger> logger_;  ///< @brief Application logger
std::atomic<ServerState> state_{ServerState::NotStarted};  ///< @brief Atomic server state
```

Или многострочный если требуется контекст:

```cpp
/** @brief Route registry: pattern → HTTP method → handler */
std::map<std::string, std::map<std::string, std::shared_ptr<IHttpHandler>>> handlers_;
```

### Enum

Enum class — `@enum` + `@brief`, каждое значение — однострочный комментарий:

```cpp
/**
 * @enum ServerState
 * @brief Server lifecycle states
 */
enum class ServerState : uint8_t
{
    NotStarted,  ///< Not started yet
    Running,     ///< Server is running
    Stopped      ///< Server has been stopped
};
```

### Чего НЕ делать

- Не дублировать имя метода в `@brief`: `@brief handleRequest` — избыточно
- Не документировать `override`-методы если они не добавляют семантики по сравнению с базовым классом (но документируй если поведение отличается)
- Не ставить `@author` на каждый метод — только на файл
- Не использовать `///` вместо `/** */` — стиль проекта: Qt-стиль (`/** */`)

## Предкоммитная проверка чистоты кода

Перед каждым коммитом запускать clang-tidy на изменённых файлах:
```bash
clang-tidy -p build --warnings-as-errors='*' <изменённые файлы>
```
Если есть замечания — исправить перед коммитом. clang-tidy конфиг в `.clang-tidy`.

Известные подавления (в `.clang-tidy`): `pro-type-member-init`, `special-member-functions`, `use-nodiscard`, `enum-size`, `convert-member-functions-to-static`, `implicit-bool-conversion`, `named-parameter`, `easily-swappable-parameters`.