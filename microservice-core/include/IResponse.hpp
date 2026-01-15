#pragma once
#include <string>
#include <map>
#include <optional>

/**
 * @file IResponse.hpp
 * @brief Интерфейс HTTP-ответа
 * @version 2.0
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
     *   // JSON успешный ответ
     *   res.setResult(200, "application/json", R"({"status": "ok"})");
     * 
     *   // JSON ошибка
     *   res.setResult(404, "application/json", R"({"error": "Not found"})");
     */
    virtual void setResult(int code, 
                           const std::string& contentType, 
                           const std::string& body) = 0;
};
