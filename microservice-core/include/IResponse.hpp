#pragma once
#include <string>
#include <map>
#include <optional>
#include "HttpStatus.hpp"

/**
 * @file IResponse.hpp
 * @brief Интерфейс HTTP-ответа
 * @version 2.1
 * @author Anton Tobolkin
 */
struct IResponse {
    virtual ~IResponse() = default;

    // =========================================================================
    // SETTERS — установка данных ответа
    // =========================================================================

    /**
     * @brief Установить HTTP статус код
     * @param code Код статуса (200, 201, 400, 401, 404, 500, etc.)
     */
    virtual void setStatus(int code) = 0;

    /**
     * @brief Установить HTTP статус из HttpStatus enum
     * @param status HttpStatus enum value
     */
    virtual void setStatus(HttpStatus status) = 0;

    /**
     * @brief Установить тело ответа
     * @param body Тело ответа
     */
    virtual void setBody(const std::string& body) = 0;

    /**
     * @brief Установить заголовок ответа
     * @param name Имя заголовка
     * @param value Значение заголовка
     */
    virtual void setHeader(const std::string& name, const std::string& value) = 0;

    /**
     * @brief Установить Set-Cookie заголовок
     * @param name Имя cookie
     * @param value Значение cookie
     * @param path Path attribute (default: "/")
     * @param httpOnly HttpOnly flag (default: true)
     * @param secure Secure flag (default: false)
     * @param maxAge Max-Age in seconds (default: -1 = not set)
     */
    virtual void setCookie(const std::string& name,
                            const std::string& value,
                            const std::string& path = "/",
                            bool httpOnly = true,
                            bool secure = false,
                            int maxAge = -1) = 0;

    // =========================================================================
    // GETTERS — получение данных ответа
    // =========================================================================

    /**
     * @brief Получить HTTP статус код
     * @return Код статуса
     * 
     * @note Используется для logging middleware, тестирования.
     */
    virtual int getStatus() const = 0;

    /**
     * @brief Получить тело ответа
     * @return Тело ответа
     */
    virtual std::string getBody() const = 0;

    /**
     * @brief Получить все заголовки ответа
     * @return Map имя → значение
     */
    virtual std::map<std::string, std::string> getHeaders() const = 0;

    /**
     * @brief Получить заголовок по имени
     * @param name Имя заголовка
     * @return Значение или nullopt если не установлен
     * 
     * @note Case-insensitive по HTTP стандарту.
     */
    virtual std::optional<std::string> getHeader(const std::string& name) const = 0;

    // =========================================================================
    // CONVENIENCE METHODS — удобные методы
    // =========================================================================

    /**
     * @brief Установить полный результат ответа
     * @param code HTTP статус код
     * @param contentType Значение Content-Type
     * @param body Тело ответа
     * 
     * @note Эквивалентно:
     *   setStatus(code);
     *   setHeader("Content-Type", contentType);
     *   setBody(body);
     * 
     * @example
     *   res.setResult(HttpStatus::Ok, "application/json", R"({"status": "ok"})");
     *   res.setResult(HttpStatus::NotFound, "application/json", R"({"error": "Not found"})");
     */
    virtual void setResult(int code, 
                           const std::string& contentType, 
                           const std::string& body) = 0;

    /**
     * @brief Установить полный результат ответа с HttpStatus enum
     * @param status HttpStatus enum value
     * @param contentType Значение Content-Type
     * @param body Тело ответа
     */
    virtual void setResult(HttpStatus status,
                           const std::string& contentType,
                           const std::string& body) = 0;

    // =========================================================================
    // TRACE ID — сквозная идентификация запроса
    // =========================================================================

    /**
     * @brief Установить X-Trace-ID в ответ
     * @param id Trace ID
     * 
     * @note Обычно вызывается ChainHandler автоматически после обработки цепочки.
     */
    virtual void setTraceId(const std::string& id) = 0;
};
