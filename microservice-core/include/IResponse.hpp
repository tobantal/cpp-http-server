#pragma once
#include <string>

/**
 * @file IResponse.hpp
 * @brief Интерфейс HTTP-ответа
 * @author Anton Tobolkin
 */
struct IResponse {
    virtual ~IResponse() = default;

    /**
     * @brief Установить статус ответа (например, 200, 404)
     */
    virtual void setStatus(int code) = 0;
    /**
     * @brief Установить тело ответа
     */
    virtual void setBody(const std::string& body) = 0;

    /**
     * @brief Установить заголовок ответа
     */
    virtual void setHeader(const std::string& name, const std::string& value) = 0;

};
