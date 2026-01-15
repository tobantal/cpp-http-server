#pragma once
#include <string>
#include <map>
#include <vector>
#include <optional>


/**
 * @file IRequest.hpp
 * @brief Интерфейс HTTP-запроса
 * @version 2.0
 * @author Anton Tobolkin
 */
struct IRequest {
    virtual ~IRequest() = default;


    // =========================================================================
    // PATH — работа с путём запроса
    // =========================================================================


    /**
     * @brief Получить путь запроса БЕЗ query string
     * @return Путь, например "/api/v1/orders"
     * 
     * @example
     *   Запрос: GET /api/v1/orders?status=active
     *   getPath() → "/api/v1/orders"
     * 
     * @contract Всегда возвращает путь без query string.
     */
    virtual std::string getPath() const = 0;


    /**
     * @brief Получить сегменты пути
     * @return Вектор сегментов (без пустых элементов)
     * 
     * @example
     *   Путь: /api/v1/orders/ord-123
     *   getPathSegments() → ["api", "v1", "orders", "ord-123"]
     * 
     * @example
     *   Путь: /
     *   getPathSegments() → []
     */
    virtual std::vector<std::string> getPathSegments() const = 0;


    // =========================================================================
    // PATH PARAMETERS — параметры из URL паттерна
    // =========================================================================


    /**
     * @brief Получить паттерн, по которому был найден handler
     * @return Паттерн или пустая строка если не установлен
     * 
     * @example
     *   Паттерн: "/api/v1/orders/ *" (wildcard)
     *   getPathPattern() возвращает "/api/v1/orders/ *"
     * 
     * @note Устанавливается в BoostBeastApplication::handleRequest() 
     *       после матчинга маршрута.
     */
    virtual std::string getPathPattern() const = 0;


    /**
     * @brief Установить паттерн маршрута
     * @param pattern Паттерн с wildcards
     * 
     * @note Вызывается в BoostBeastApplication::handleRequest()
     *       после успешного матчинга в findHandler().
     */
    virtual void setPathPattern(const std::string& pattern) = 0;


    /**
     * @brief Получить path parameter по индексу wildcard
     * @param index Индекс wildcard в паттерне (начиная с 0)
     * @return Значение параметра или nullopt если индекс вне диапазона
     * 
     * @example
     *   Паттерн: "/api/v1/orders/ *"
     *   Путь:    "/api/v1/orders/ord-123"
     *   getPathParam(0) возвращает "ord-123"
     * 
     * @example
     *   Паттерн: "/api/v1/orders/ * /items/ *"
     *   Путь:    "/api/v1/orders/ord-123/items/item-456"
     *   getPathParam(0) возвращает "ord-123"
     *   getPathParam(1) возвращает "item-456"
     *   getPathParam(2) возвращает nullopt
     * 
     * @note Вычисляется на основе getPath() и getPathPattern().
     */
    virtual std::optional<std::string> getPathParam(size_t index) const = 0;


    // =========================================================================
    // QUERY PARAMETERS — параметры из query string
    // =========================================================================


    /**
     * @brief Получить все query parameters
     * @return Map имя → значение
     * 
     * @example
     *   Запрос: GET /orders?status=active&limit=10
     *   getQueryParams() → {"status": "active", "limit": "10"}
     * 
     * @note Переименован из getParams() для ясности.
     */
    virtual std::map<std::string, std::string> getQueryParams() const = 0;


    /**
     * @brief Получить query parameter по имени
     * @param name Имя параметра
     * @return Значение или nullopt если не найден
     * 
     * @example
     *   Запрос: GET /orders?status=active
     *   getQueryParam("status") → "active"
     *   getQueryParam("limit") → nullopt
     */
    virtual std::optional<std::string> getQueryParam(const std::string& name) const = 0;


    /**
     * @brief Установить query parameter
     * @param name Имя параметра
     * @param value Значение параметра
     */
    virtual void setQueryParam(const std::string& name, const std::string& value) = 0;


    /**
     * @deprecated Используйте getQueryParams()
     * @brief Алиас для обратной совместимости
     */
    virtual std::map<std::string, std::string> getParams() const {
        return getQueryParams();
    }


    // =========================================================================
    // HEADERS — HTTP заголовки
    // =========================================================================


    /**
     * @brief Получить все HTTP заголовки
     * @return Map имя → значение
     */
    virtual std::map<std::string, std::string> getHeaders() const = 0;


    /**
     * @brief Получить значение заголовка по имени
     * @param name Имя заголовка
     * @return Значение или nullopt если не найден
     * 
     * @note Case-insensitive по HTTP стандарту (RFC 7230).
     *       getHeader("Content-Type") == getHeader("content-type")
     */
    virtual std::optional<std::string> getHeader(const std::string& name) const = 0;


    /**
     * @brief Установить заголовок
     * @param name Имя заголовка
     * @param value Значение заголовка
     */
    virtual void setHeader(const std::string& name, const std::string& value) = 0;


    /**
     * @brief Установить несколько заголовков
     * @param headers Map имя → значение
     * 
     * @note Существующие заголовки с такими же именами перезаписываются.
     */
    virtual void setHeaders(const std::map<std::string, std::string>& headers) = 0;


    // =========================================================================
    // BODY — тело запроса
    // =========================================================================


    /**
     * @brief Получить тело запроса
     * @return Тело запроса как строка
     */
    virtual std::string getBody() const = 0;


    /**
     * @brief Установить тело запроса
     * @param body Тело запроса
     */
    virtual void setBody(const std::string& body) = 0;


    // =========================================================================
    // METHOD — HTTP метод
    // =========================================================================


    /**
     * @brief Получить HTTP метод
     * @return Метод в верхнем регистре (GET, POST, PUT, DELETE, PATCH, etc.)
     */
    virtual std::string getMethod() const = 0;


    // =========================================================================
    // CONNECTION INFO — информация о соединении
    // =========================================================================


    /**
     * @brief Получить IP-адрес
     * @return IP клиента (для входящих) или целевой IP (для исходящих)
     */
    virtual std::string getIp() const = 0;


    /**
     * @brief Получить порт
     * @return Порт (80 по умолчанию для входящих, целевой для исходящих)
     */
    virtual int getPort() const = 0;


    // =========================================================================
    // CONVENIENCE METHODS — удобные методы
    // =========================================================================


    /**
     * @brief Извлечь Bearer token из заголовка Authorization
     * @return Token без префикса "Bearer " или nullopt
     * 
     * @example
     *   Header: Authorization: Bearer eyJ...
     *   getBearerToken() → "eyJ..."
     * 
     *   Header: Authorization: Basic abc123
     *   getBearerToken() → nullopt
     */
    virtual std::optional<std::string> getBearerToken() const = 0;


    /**
     * @brief Проверить, является ли Content-Type JSON
     * @return true если Content-Type содержит "json"
     */
    virtual bool isJson() const = 0;


    /**
     * @brief Получить Content-Type
     * @return Значение Content-Type или пустая строка если не установлен
     */
    virtual std::string getContentType() const = 0;


    // =========================================================================
    // ATTRIBUTES — передача данных между middleware/handlers
    // =========================================================================


    /**
     * @brief Установить атрибут запроса
     * @param name Имя атрибута
     * @param value Значение атрибута
     * 
     * @example
     *   // В AuthMiddleware после валидации токена:
     *   req.setAttribute("user_id", "user-123");
     */
    virtual void setAttribute(const std::string& name, const std::string& value) = 0;


    /**
     * @brief Получить атрибут запроса
     * @param name Имя атрибута
     * @return Значение или nullopt если не установлен
     */
    virtual std::optional<std::string> getAttribute(const std::string& name) const = 0;
};
