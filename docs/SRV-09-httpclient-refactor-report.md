# SRV-09: HttpClient::send() — Отчёт о рефакторинге

> Дата: 2026-04-15
> Статус: На рассмотрении

## 1. Текущие проблемы

| # | Проблема | Где | Серьёзность |
|---|---------|-----|-------------|
| **1** | **`bool` return type — потеря информации об ошибке** | `IHttpClient::send()` | P0 |
| **2** | **Смешивание sync + async в одной функции** | `send()` — resolve sync, connect async, write/read sync | P1 |
| **3** | **Нет read/write timeout** | `http::write()` и `http::read()` — без таймаутов | P1 |
| **4** | **Новый io_context + resolver на каждый запрос** | `send()` — строка 34-35 | P1 |
| **5** | **Мутация чужого IResponse при ошибке** | `response.setStatus(500)` + `setBody("Internal Server Error")` — caller не отличает ошибку сети от HTTP 500 от сервера | P0 |
| **6** | **God function — 100 строк, 6 обязанностей** | `send()` делает resolve, connect, build request, write, read, fill response | P2 |

## 2. Детальный разбор

### 2.1 `bool` → `HttpClientResult` (P0)

Текущая сигнатура `bool send(request, response)` нарушает правило из error-handling.md:
> *«В перспективе: добавить error code enum (ConnectionFailed, Timeout, DnsError)»*

Caller не может отличить:
- DNS failed (хост не найден)
- Connect timeout (сервер не отвечает)
- Connection refused (порт закрыт)
- Read/write timeout
- HTTP 500 от удалённого сервера (это корректный ответ, а не ошибка клиента!)

**Предложение:**

```cpp
enum class HttpClientError : uint8_t {
    None,
    DnsFailed,
    ConnectTimeout,
    ConnectionRefused,
    WriteTimeout,
    ReadTimeout,
    TlsError,
    UnknownError
};

struct HttpClientResult {
    HttpClientError error = HttpClientError::None;
    std::string errorMessage;
    bool ok() const { return error == HttpClientError::None; }
};
```

Новый интерфейс:
```cpp
virtual HttpClientResult send(const IRequest& request, IResponse& response) = 0;
```

**Backward compatibility:** `bool` → `HttpClientResult` — breaking change. Consumer-код: `if (client.send(req, res))` → `if (client.send(req, res).ok())`. Нужно обновить trading-platform.

**Плюс:** IResponse теперь заполняется **только при успешном получении HTTP-ответа**. При сетевой ошибке `response` не мутируется — caller проверяет `result.ok()` и читает `result.error`.

### 2.2 Проблема 5: IResponse mutation при сетевой ошибке

Сейчас при connect timeout / DNS error:
```cpp
response.setStatus(500);
response.setBody("Internal Server Error");
return false;
```

Caller не может отличить «сервер вернул HTTP 500» от «соединение не установлено». **Решение:** при `HttpClientError != None` — не трогать `response`. Caller знает: `response` валиден только если `result.ok() == true`.

### 2.3 Единообразный async (P1)

Сейчас мешанина: resolve sync → connect async → write/read sync. Это порождает проблему: connect timeout через async+timer, а read/write вообще без timeout.

**Вариант A — всё async:** `async_resolve` → `async_connect` → `async_write` → `async_read`, всё на одном `io_context` с `stream.expires_after()` между фазами. Самый правильный, но требует переписать `send()` целиком.

**Вариант B — всё sync + `beast::tcp_stream` таймауты:** После `ioc.run()` (connect) переставить `stream.expires_after(readTimeout_)` перед `http::read()`. Минимальные изменения.

**Рекомендация:** Вариант B как шаг 1 (быстро, закрывает read/write timeout). Вариант A — на будущее, если нужен async API.

### 2.4 io_context per request (P1)

```cpp
asio::io_context ioc;       // новый на каждый send()
tcp::resolver resolver(ioc); // новый resolver
```

Для единичных запросов это ок. Для connection pooling (будущее) — нужен общий `io_context` + `tcp_stream` реюз. Сейчас оставляем как есть, но отмечаем в TODO.

### 2.5 God function → композиция (P2)

`send()` = resolve + connect + build + write + read + fill. Вынести в приватные методы:

```cpp
HttpClientResult connect(beast::tcp_stream& stream, const std::string& host, const std::string& port);
HttpClientResult sendRequest(beast::tcp_stream& stream, const IRequest& request);
HttpClientResult readResponse(beast::tcp_stream& stream, IResponse& response);
```

## 3. План и оценки

| Шаг | Что | SP | Breaking? |
|-----|-----|----|-----------|
| **1** | `HttpClientError` enum + `HttpClientResult` | 1 | **Да** — IHttpClient.send() сигнатура |
| **2** | `send()` возвращает `HttpClientResult`, не мутирует IResponse при ошибке | 1 | **Да** — caller-код |
| **3** | Read/write timeout через `stream.expires_after()` | 1 | Нет |
| **4** | Декомпозиция send() в приватные методы | 1 | Нет |
| **5** | Настройки: `HTTP_CLIENT_READ_TIMEOUT_MS`, `HTTP_CLIENT_WRITE_TIMEOUT_MS` | 0.5 | Нет |
| | **Итого** | **4.5** | |

Шаги 1-2 можно объединить в один коммит (breaking change — одна миграция). Шаги 3-5 — backward compatible.

## 4. Миграция для consumer-проектов

```cpp
// БЫЛО (trading-platform):
bool ok = client.send(req, res);
if (!ok) { return 500; }

// СТАЛО:
auto result = client.send(req, res);
if (!result.ok()) {
    // result.error → HttpClientError::ConnectTimeout и т.д.
    // result.errorMessage → "timeout" / "Connection refused" / ...
    return mapToHttpStatus(result.error);
}
// res.getStatus() — реальный HTTP-статус от сервера (200, 404, 500...)
```

Главная выгода: **caller различает «сервер ответил 500» от «сервер недоступен»**, что критично для circuit breaker / retry логики в микросервисах.