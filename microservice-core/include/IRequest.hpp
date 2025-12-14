#pragma once
#include <string>
#include <map>

/**
 * @file IRequest.hpp
 * @brief Интерфейс HTTP-запроса
 * @author Anton Tobolkin
 */
struct IRequest {
    virtual ~IRequest() = default;

    /**
     * @brief Получить путь запроса
     */
    virtual std::string getPath() const = 0;
    /**
     * @brief Получить метод запроса (GET, POST, и т.д.)
     */
    virtual std::string getMethod() const = 0;
    /**
     * @brief Получить тело запроса
     */
    virtual std::string getBody() const = 0;

    /**
     * @brief Получить параметры запроса в виде ключ-значение
     */
    virtual std::map<std::string, std::string> getParams() const = 0;

    /**
     * @brief Получить HTTP-заголовки запроса
     */
    virtual std::map<std::string, std::string> getHeaders() const = 0;

    /**
     * @brief Получить IP-адрес (клиента при входящем сообщении, получателя при исходящем)
     */
    virtual std::string getIp() const = 0;

    /**
     * @brief Получить порт (для входящих - 80 по умолчанию, для исходящих - целевой порт)
     */
    virtual int getPort() const = 0;
};
