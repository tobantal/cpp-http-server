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
- Интерфейсы и мелкие реализации — header-only (SimpleRequest, SimpleResponse)
- Крупные реализации — отдельный .cpp (BoostBeastApplication, HttpClient, RouteMatcher)
- При росте header-only файла > 150-200 строк — вынести реализацию в .cpp

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

## Кодогенерация и DRY

- Дублирование `toLower()` и `splitPath()` в 4 файлах — вынести в общий utility (`StringUtils.hpp` в core)
- Дублирование `getPathParam()` логики — вынести в общий helper или mixin