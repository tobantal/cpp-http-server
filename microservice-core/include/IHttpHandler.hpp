#pragma once

#include "IRequest.hpp"
#include "IResponse.hpp"

/**
 * @file IHttpHandler.hpp
 * @brief Интерфейс обработчика HTTP запросов
 * @author Anton Tobolkin
 */

/**
 * @class IHttpHandler
 * @brief Интерфейс для обработки HTTP запросов
 * 
 * Каждый handler отвечает за обработку конкретного эндпоинта.
 * Регистрируется в IoC с именем вида "Метод:Путь", например, "GET:/api/users"
 */
class IHttpHandler
{
public:
    virtual ~IHttpHandler() = default;

    /**
     * @brief Обработать HTTP запрос
     * @param req HTTP запрос
     * @param res HTTP ответ
     */
    virtual void handle(IRequest& req, IResponse& res) = 0;
};